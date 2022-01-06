%% this is used to extract features from different layers of the xiaoqian's model
function preMat=test_3dBrainFCN(subMatT1,subMatT2)
% clear 
% clc
addpath('../+caffe');

% mat=importdata('BFI44_O.txt');
% data=mat(:,1:end-1);
% labels=mat(:,end);

% y=zeros(size(data,1),128,1);
% for i=1:size(data,1)
im_data={subMatT1,subMatT2};
model.dir='../../examples/infantBrain32/';
model.deployfilename = 'infant_deploy.prototxt';
model.modelfilename = 'infant_fcn_iter_300000.caffemodel';
score=classification_demo(im_data,1,model);
preMat=max4dMatrix(score);
%[score,class]=max(score);
% end
% yhat=squeeze(y);
% yhat=y;
return