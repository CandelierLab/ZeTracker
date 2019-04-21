clc

warning('off', 'images:imshow:magnificationMustBeFitForDockedFigure');

% === Parameters ==========================================================

imgPath = '/home/ljp/Science/ZeBeauty/Data/Test/';

th = 0.3;

% =========================================================================

% --- Load Background
if ~exist('Bkg', 'var')
    Bkg = double(imread([imgPath 'Background.pgm']));
    Bkg = fliplr(Bkg);
end

Mon = NaN(256,256,8);

for i = 1:8

    % --- Load image
    Tmp = imread([imgPath 'img_' num2str(i) '.png']);
    Img = imresize(mean(Tmp,3), size(Bkg));
    
%     Res = imbinarize(Img/255, 'adaptive', 'ForegroundPolarity','bright','Sensitivity',0.7);
    
    Res = double(edge(Img, 'Roberts'))>th;
%     
%     Res = Img - Bkg;
%     Res = imdilate(Res, strel('disk', 2));
%     Res = imgaussfilt(Res, [1 1]*3);
%     Res = Res>th;
    
    Mon(:,:,i) = Res;
    
end

% --- Display -------------------------------------------------------------

clf

subplot(2,1,1)
imshow(Img)
caxis auto

subplot(2,1,2)
imshow(Res)
caxis auto

% 
% montage(Mon)
% caxis auto %([0 100])
% % caxis([0 1])
% colorbar