#include "FLIR.h"

/* === Constructor =================================================== */

Camera_FLIR::Camera_FLIR(class Vision* V):Vision(V) {

    // Initialization
    grabState = false;

    // Camera initialization
    FLIR_system = Spinnaker::System::GetInstance();
    FLIR_camList = FLIR_system->GetCameras();
    unsigned int FLIR_nCam = FLIR_camList.GetSize();

    if (FLIR_nCam) {

        pCam = FLIR_camList.GetByIndex(0);

        // --- Get camera identifier
        Spinnaker::GenApi::INodeMap &nodeMap = pCam->GetTLDeviceNodeMap();
        Spinnaker::GenApi::CCategoryPtr category = nodeMap.GetNode("DeviceInformation");
        if (IsAvailable(category) && IsReadable(category)) {
            Spinnaker::GenApi::FeatureList_t features;
            category->GetFeatures(features);

            QRegExp rx(" (\\w+)\-");
            QString model("");
            if (rx.indexIn(QString(features.at(3)->ToString())) != -1) { model += rx.cap(1) + " "; }
            camName = model + features.at(1)->ToString();

        }

    }

}

/* === Destructor ==================================================== */

Camera_FLIR::~Camera_FLIR() { pCam->DeInit(); }

/* === Information display =========================================== */

void Camera_FLIR::sendInfo() {

    // --- Device info

    Spinnaker::GenApi::INodeMap &nodeMap = pCam->GetTLDeviceNodeMap();
    Spinnaker::GenApi::CCategoryPtr category = nodeMap.GetNode("DeviceInformation");

    if (IsAvailable(category) && IsReadable(category)) {

        Spinnaker::GenApi::FeatureList_t features;
        category->GetFeatures(features);

        qInfo().nospace() << "<table class='cameraInfo'>"
                          << "<tr>"
                          << "<th rowspan=3>Camera " << 0 << "</th>"
                          << "<td>" << features.at(1)->GetName() << "</td>"       // DeviceSerialNumber
                          << "<td>" << features.at(1)->ToString() << "</td>"
                          << "</tr><tr>"
                          << "<td>" << features.at(3)->GetName() << "</td>"       // DeviceModelName
                          << "<td>" << features.at(3)->ToString() << "</td>"
                          << "</tr><tr>"
                          << "<td>" << features.at(12)->GetName() << "</td>"      // DeviceCurrentSpeed
                          << "<td>" << features.at(12)->ToString() << "</td>"
                          << "</tr>"
                          << "</table>";

    } else { qWarning() << "Device info not available"; }

}

/* === Frame grabber ================================================= */

void Camera_FLIR::grab() {

    // Thread info
    qInfo().nospace() << THREAD << qPrintable(camName) << " thread: " << QThread::currentThreadId();
    qInfo() << THREAD << "Priority:" << QThread::currentThread()->priority();

    // --- Camera & nodemaps definitions -----------------------------------

    pCam->Init();
    Spinnaker::GenApi::INodeMap &nodeMap = pCam->GetNodeMap();
    pCam->GainAuto.SetValue(Spinnaker::GainAutoEnums::GainAuto_Off);

    // --- Configure ChunkData ---------------------------------------------

    // --- Activate chunk mode

    Spinnaker::GenApi::CBooleanPtr ptrChunkModeActive = nodeMap.GetNode("ChunkModeActive");

    if (!IsAvailable(ptrChunkModeActive) || !IsWritable(ptrChunkModeActive)) {
        qWarning() << "Unable to activate chunk mode. Aborting.";
        return;
    }

    ptrChunkModeActive->SetValue(true);
    qInfo() << "Chunk mode activated";

    // --- Chunk data types

    Spinnaker::GenApi::NodeList_t entries;

    // Retrieve the selector node
    Spinnaker::GenApi::CEnumerationPtr ptrChunkSelector = nodeMap.GetNode("ChunkSelector");

    if (!IsAvailable(ptrChunkSelector) || !IsReadable(ptrChunkSelector)) {
        qWarning() << "Unable to retrieve chunk selector. Aborting.";
        return;
    }

    // Retrieve entries
    ptrChunkSelector->GetEntries(entries);

    for (unsigned int i = 0; i < entries.size(); i++) {

        // Select entry to be enabled
        Spinnaker::GenApi::CEnumEntryPtr ptrChunkSelectorEntry = entries.at(i);

        // Go to next node if problem occurs
        if (!IsAvailable(ptrChunkSelectorEntry) || !IsReadable(ptrChunkSelectorEntry)) { continue; }

        ptrChunkSelector->SetIntValue(ptrChunkSelectorEntry->GetValue());

        // Retrieve corresponding boolean
        Spinnaker::GenApi::CBooleanPtr ptrChunkEnable = nodeMap.GetNode("ChunkEnable");

        // Enable the boolean, thus enabling the corresponding chunk data
        if (IsWritable(ptrChunkEnable)) {
            ptrChunkEnable->SetValue(true);
        }

    }

    // --- Acquisition parameters ------------------------------------------

    // === Continuous mode ======================

    Spinnaker::GenApi::CEnumerationPtr ptrAcquisitionMode = nodeMap.GetNode("AcquisitionMode");
    if (!IsAvailable(ptrAcquisitionMode) || !IsWritable(ptrAcquisitionMode)) {
        qWarning() << "Unable to set acquisition mode to continuous (enum retrieval)";
        return;
    }

    // Retrieve entry node from enumeration node
    Spinnaker::GenApi::CEnumEntryPtr ptrAcquisitionModeContinuous = ptrAcquisitionMode->GetEntryByName("Continuous");
    if (!IsAvailable(ptrAcquisitionModeContinuous) || !IsReadable(ptrAcquisitionModeContinuous)) {
        qWarning() << "Unable to set acquisition mode to continuous (entry retrieval)";
        return;
    }

    // Retrieve integer value from entry node
    int64_t acquisitionModeContinuous = ptrAcquisitionModeContinuous->GetValue();

    // Set integer value from entry node as new value of enumeration node
    ptrAcquisitionMode->SetIntValue(acquisitionModeContinuous);

    qInfo() << "Acquisition mode set to continuous";

    // === Exposure time ========================

    // Disable auto exposure
    Spinnaker::GenApi::CEnumerationPtr exposureAuto = nodeMap.GetNode("ExposureAuto");
    exposureAuto->SetIntValue(exposureAuto->GetEntryByName("Off")->GetValue());

    Spinnaker::GenApi::CEnumerationPtr exposureMode = nodeMap.GetNode("ExposureMode");
    exposureMode->SetIntValue(exposureMode->GetEntryByName("Timed")->GetValue());

    // Ensure that exposure time does not exceed the maximum
    Spinnaker::GenApi::CFloatPtr ExposureTime = nodeMap.GetNode("ExposureTime");
    const double ExposureMax = ExposureTime->GetMax();
    if (Vision->exposure > ExposureMax) {
        qInfo().nospace() << "Exposure limited by max value (" << ExposureMax/1000 << " ms)";
        Vision->exposure = ExposureMax;
    }

    // Apply exposure time
    ExposureTime->SetValue(Vision->exposure);

    emit Vision->updateExposure();
    qInfo() << "Exposure time set to " << double(Vision->exposure)/1000 << "ms";

    // === Framerate ===========================
/*
    CFloatPtr ptrAcquisitionFrameRate = nodeMap.GetNode("AcquisitionFrameRate");
    if (IsAvailable(ptrAcquisitionFrameRate) && IsWritable(ptrAcquisitionFrameRate)){
      ptrAcquisitionFrameRate->SetValue(frameRate);
      qInfo() << "Framerate set to " << frameRate << "fps";
    }
*/
    // === Image size ===========================

    Spinnaker::GenApi::CIntegerPtr pWidth = nodeMap.GetNode("Width");
    if (IsAvailable(pWidth) && IsWritable(pWidth)) { pWidth->SetValue(width); }

    Spinnaker::GenApi::CIntegerPtr pHeight = nodeMap.GetNode("Height");
    if (IsAvailable(pHeight) && IsWritable(pHeight)) { pHeight->SetValue(height); }

    Spinnaker::GenApi::CIntegerPtr pOffsetX = nodeMap.GetNode("OffsetX");
    if (IsAvailable(pOffsetX) && IsWritable(pOffsetX)) { pOffsetX->SetValue(offsetX); }

    Spinnaker::GenApi::CIntegerPtr pOffsetY = nodeMap.GetNode("OffsetY");
    if (IsAvailable(pOffsetY) && IsWritable(pOffsetY)) { pOffsetY->SetValue(offsetY); }

    // --- Acquire images --------------------------------------------------

    qInfo() << "Starting image acquisition";
    grabState = true;
    timer.start();

    pCam->BeginAcquisition();

    while (grabState) {

        Spinnaker::ImagePtr pImg = pCam->GetNextImage();

        if (pImg->IsIncomplete()) {

            qWarning() << "Image incomplete with image status " << pImg->GetImageStatus();

        } else {

            // --- Get image
            unsigned char* Raw = (unsigned char*)pImg->GetData();
            Mat M(height, width, CV_8UC1, Raw);

            // Binning to 256x256
            resize(M, M, Size(), 0.5, 0.5, INTER_AREA);
            transpose(M,M);
            flip(M,M,0);
            // flip(M,M,1);

            Frame F;
            M.getUMat(ACCESS_RW).copyTo(F.img);

            // --- Get ChunkData
            Spinnaker::ChunkData chunkData = pImg->GetChunkData();
            F.timestamp = timer.nsecsElapsed(); //(qint64) chunkData.GetTimestamp();
            F.frameId = (qint64) chunkData.GetFrameID();

            emit newFrame(F);
        }

        pImg->Release();

    }

    pCam->EndAcquisition();
    qInfo() << "Camera stopped.";

}
