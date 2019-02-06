% Add the entire phantast/MATLAB directory as well as its
% sub-directories are in the MATLAB path
addContainingDirAndSubDir()
rootFolder_train = 'F:\MA_Yang\code\data\crop_phantast_train\';
saveFolder_train = 'F:\MA_Yang\code\data\crop_phantast_train\';
rootFolder_val = 'F:\MA_Yang\code\data\crop_phantast_val\';
saveFolder_val = 'F:\MA_Yang\code\data\crop_phantast_val\';
generate_mat(rootFolder_train, saveFolder_train);
generate_mat(rootFolder_val, saveFolder_val);