clc

warning('off', 'images:imshow:magnificationMustBeFitForDockedFigure');

% === Parameters ==========================================================

imgPath = '/home/ljp/Science/ZeBeauty/Data/Test/';

size_mask = 20;
size_eye = 4;
size_yolk = [2 3];

i = 4;

% =========================================================================

% --- Load Background
if ~exist('Bkg', 'var')
    Bkg = double(imread([imgPath 'Background.pgm']));
    Bkg = fliplr(Bkg);
end

% --- Load image
Tmp = imread([imgPath 'img_' num2str(i) '.png']);
Img = imresize(mean(Tmp,3), size(Bkg));
    
Res = Img - Bkg;

% --- Find the eyes

% Create Masks
[X,Y] = meshgrid(1:size_mask,1:size_mask);
Meye = ((X-size_mask/2).^2  + (Y-size_mask/2).^2)<=(size_eye/2)^2;
% Meye = imgaussfilt(double(Meye), 1);

% First eye
C1 = conv2(-Res, Meye, 'same');
[e1y,e1x] = ind2sub(size(Img), find(C1==max(C1(:))));
Res(round(e1y+(-size_eye/2:size_eye/2)), round(e1x+(-size_eye/2:size_eye/2))) = 255;

% Second eye
C2 = conv2(-Res, Meye, 'same');
[e2y,e2x] = ind2sub(size(Img), find(C2==max(C2(:))));
Res(round(e2y+(-size_eye/2:size_eye/2)), round(e2x+(-size_eye/2:size_eye/2))) = 255;

% --- Find the yolk

% Create mask
Myolk = ((X-size_mask/2).^2  + (Y-size_mask/2).^2)>=(size_yolk(1)/2)^2 & ...
    ((X-size_mask/2).^2  + (Y-size_mask/2).^2)<=(size_yolk(2)/2)^2;
Myolk = imgaussfilt(double(Myolk), 1);

% Myolk = exp(-((X-size_mask/2).^2  + (Y-size_mask/2).^2)*2/size_yolk(2)^2) - ...
%     exp(-((X-size_mask/2).^2  + (Y-size_mask/2).^2)*2/size_yolk(1)^2);

% Convolution
C3 = conv2(-Res, Myolk, 'same');
[y0y,y0x] = ind2sub(size(Img), find(C3==max(C3(:))));

Res = (Img - Bkg);

% --- Display -------------------------------------------------------------

clf
hold on

% imshow(Myolk)

imshow(Res)
scatter(e1x,e1y);
scatter(e2x,e2y);
scatter(y0x,y0y, '+');

caxis auto
% caxis([0 1])
colorbar
axis image on
