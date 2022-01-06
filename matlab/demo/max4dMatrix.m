%to compare the last dimension of a 3-d matrix, which returns a 2d matrix,
%each element represents a predicted label for this pixel
%input: 3d matrix, the 3rd dimension is which we want to compare
%output: 2d matrix, the result for comparing the 3rd dimension 
function maxLabels=max4dMatrix(mat)
[m,n,c]=size(mat);
maxLabels=zeros(m,n);
if length(size(mat))~=4
        fprintf('the dimension of matrix is not 3\n');
        return;
end

newMat=permute(mat,[4,1,2,3]);%transpose for high dimension
[C,maxLabels]=max(newMat);
maxLabels=squeeze(maxLabels)-1;
return


