function [csfDR,gmDR,wmDR] =computeDR(matFA,matGT)
csfA=find(matFA==1);
csfB=find(matGT==1);
csfI=intersect(csfA,csfB);%as index has the same location in both matrix
csfDR=2*length(csfI)/(length(csfA)+length(csfB));


%for gm
gmA=find(matFA==2);
gmB=find(matGT==2);
gmI=intersect(gmA,gmB);%as index has the same location in both matrix
gmDR=2*length(gmI)/(length(gmA)+length(gmB));

%for wm
wmA=find(matFA==3);
wmB=find(matGT==3);
wmI=intersect(wmA,wmB);%as index has the same location in both matrix
wmDR=2*length(wmI)/(length(wmA)+length(wmB));
return