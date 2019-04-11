#include "Interface.h"
#include "mainwindow.h"

/* ====================================================================== *
 *      CONSTRUCTOR                                                       *
 * ====================================================================== */

Interface::Interface(MainWindow *MW, class Motion *M, class Vision *V)
    :Main(MW), Motion(M), Vision(V) {

    // === CONNECTIONS =====================================================

    connect(Vision, SIGNAL(updateFish()), this, SLOT(updateFish()));

}

/* ====================================================================== *
 *      UPDATES                                                           *
 * ====================================================================== */

void Interface::updateFish() {

    // --- GUI -------------------------------------------------------------

    // Update (dx,dy)
    emit updateDxy();

    // Update curvature
    Main->pTime.append(Vision->time*1e-9);
    Main->pCurv.append(Vision->fish.curvature*1000/Vision->pix2mm);

    while (Main->pTime.size() > Main->maxLengthCurv) {
        Main->pTime.pop_front();
        Main->pCurv.pop_front();
    }

    // --- Define bout ? ---------------------------------------------------




}
