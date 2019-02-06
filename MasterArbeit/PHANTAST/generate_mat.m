function [] = generate_mat(rootFolder, saveFolder)
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here
    files = dir(rootFolder);
    dirFlags = [files.isdir];
    subFolders = files(dirFlags);
    for j = 1 : length(subFolders)
        fprintf('Sub folder #%d = %s\n', j, subFolders(j).name);
        curFolder = strcat(rootFolder,subFolders(j).name);
        filepattern = fullfile(curFolder, '*.mat');
        theFiles = dir(filepattern);
        for k = 1 : length(theFiles)
            baseFileName = theFiles(k).name;
            if strfind(baseFileName, 'phantast')
                continue;
            else
                fullFileName = fullfile(curFolder, baseFileName);
                fprintf(1, 'Now reading %s\n', fullFileName);
                loader = load(fullFileName);
                I = loader.img;

                % Compute the local contrast image (sigma = 1.4) and threshold it (epsilon
                % = 0.06)
                %C = contrastStretching(I, 0.005);
                ImageLocalContrast = localContrast(I,1.4,0.03);

                % Correct for halo artefacts (fill area threshold = 300, kernel = 'kirsch',
                % smoll object removal threshold = 200, max object fraction to correct =
                % 0.3
                ImageHaloRemoval = haloRemoval(I,ImageLocalContrast,25,'kirsch',325,0.3);
                ImageRemoveSmallObject = removeSmallObjects(ImageHaloRemoval, 100);
                % Display the result of the processing
                %{
                subplot(1,4,1);
                imshow(I,[]); % Input PCM image
                subplot(1,4,2);
                imshow(J,[]); % Contrast-threshold output
                subplot(1,4,3);
                imshow(K,[]); % Halo-corrected output
                h=subplot(1,4,4);
                %}
                %[h, hIm]=displayBorderImage(I,ImageRemoveSmallObject,'white',1); % Segmentation overlaid on input PCM image        

                ImagePhantast = ImageRemoveSmallObject;
                result = computeConfluency(ImagePhantast); 
                disp(result{1,2});
                str = strrep(baseFileName, '.mat', '_phantast_result.mat');
                saveDir = strcat(saveFolder, subFolders(j).name);            
                if (~exist(saveDir, 'dir')); mkdir(saveDir); end
                saveFileName = fullfile(saveDir, str);
                save(saveFileName,'ImagePhantast');
            end
        end
    end
end

