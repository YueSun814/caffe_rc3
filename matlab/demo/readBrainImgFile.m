%the infant brain images, the are three type images: T1( xx_cbq.hdr), T2
%(xx_cbq-T2.hdr), label(xx-ls-corrected.hdr)
%I convert the label images to 0,1,2,3
function readBrainImgFile()
% instensity image
d=32;
len=13;
step=32;
rate=1/4;
path='../../../data/infantBrain/normals/';

files=dir([path,'*.hdr']);
id=0;
for i=1:3:length(files)
    labelfilename=files(i).name;
    info = analyze75info([path,labelfilename]);
    labelimg = analyze75read(info);%
    labelimg(find(labelimg>200))=3;%white matter
    labelimg(find(labelimg>100))=2;%gray matter
    labelimg(find(labelimg>4))=1;%csf
    
    t2filename=files(i+1).name;
    info = analyze75info([path,t2filename]);
    t2img = analyze75read(info);
    
    t1filename=files(i+2).name;
    info = analyze75info([path,t1filename]);
    t1img = analyze75read(info);
    
    words=regexp(t1filename,'_','split');
    word=words{1};
    word=lower(word);
    saveFilename=sprintf('%s',word);
    %crop areas
    id=id+1;
    [preMat,cnt]=cropCubic(t1img,t2img,labelimg,saveFilename,d,step,rate);
    preMat(find(t1img==0))=0;
    [csfDR(id),gmDR(id),wmDR(id)] =computeDR(preMat,labelimg)
end
% 
% t1filename='NORMAL01_cbq.hdr';
% info = analyze75info([path,t1filename]);
% t1img = analyze75read(info);
% 
% t2filename='NORMAL01_cbq-T2.hdr';
% info = analyze75info([path,t2filename]);
% t2img = analyze75read(info);
% 
% labelfilename='NORMAL01-ls-corrected.hdr';
% info = analyze75info([path,labelfilename]);
% labelimg = analyze75read(info);

return


%crop width*height*length from mat,and stored as image
%note,matData is 3 channels, matSet is 1 channel
%d: the patch size
function [preMat,cubicCnt]=cropCubic(matT1,matT2,matSeg,saveFilename,d,step,rate)   
    %dataT1Path='T1_64/';
    %dataT2Path='T2_64/';
    %segPath='Seg_16/';
    preMat=zeros(size(matSeg));
    eps=1e-2;
    if nargin<6
    	rate=1/4;
    end
    if nargin<5
        step=4;
    end
    if nargin<4
        d=16;
    end
    [row,col,len]=size(matT1);
    %[rowData,colData,lenData]=size(matT1);
   
    %if row~=rowData||col~=colData||len~=lenData
     %   fprintf('size of matData and matSeg is not consistent\n');
     %   exit
    %end
    cubicCnt=0;
    frameCnt=0;
    segFrameCnt=0;
%     dirname=sprintf('%s/',saveFilename);
    %mkdir([segPath,dirname]);
    %mkdir([dataT1Path,dirname]);
    %mkdir([dataT2Path,dirname]);
    %fidT1=fopen('dataT1_64.lst','a');
    %fidT2=fopen('dataT2_64.lst','a');
    %fidSeg=fopen('dataSeg_16.lst','a');
    for i=1:step:row-d+1
        for j=1:step:col-d+1
            for k=1:step:len-d+1%there is no overlapping in the 3rd dim
                dseg=floor(d*rate);
                %tempmatseg=matSeg(i+(d-dseg)/2:i+d-1-(d-dseg)/2,i+(d-dseg)/2:i+d-1-(d-dseg)/2,i+(d-dseg)/2:i+d-1-(d-dseg)/2); 
                tempmat=single(matSeg(i:i+d-1,j:j+d-1,k:k+d-1));
                tempmatT1=single(matT1(i:i+d-1,j:j+d-1,k:k+d-1));
                tempmatT2=single(matT2(i:i+d-1,j:j+d-1,k:k+d-1));
                temppremat=single(zeros(d,d,d));
                %tempmatseg=resize(tempmat,[16 16 16]);
                if sum(tempmatT1(:))<eps%all zero submat
                    preMat(i:i+d-1,j:j+d-1,k:k+d-1)=temppremat;
                    continue;
                end
                temppremat=test_3dBrainFCN(tempmatT1,tempmatT2);
                preMat(i:i+d-1,j:j+d-1,k:k+d-1)=temppremat;
                cubicCnt=cubicCnt+1;
%                 labs=matSeg(round((i+i+d-1)/2),round((j+j+d-1)/2),round((k+k+d-1)/2));
                %t1str=['../../data/infantBrain32/T1_64/',dirname,' ',sprintf('%d',frameCnt+1),' ',sprintf('%d\n',labs)];
                %fprintf(fidT1,t1str);
                %t2str=['../../data/infantBrain32/T2_64/',dirname,' ',sprintf('%d',frameCnt+1),' ',sprintf('%d\n',labs)];
                %fprintf(fidT2,t2str);
                %segstr=['../../data/infantBrain32/Seg_16/',dirname,' ',sprintf('%d',segFrameCnt+1),' ',sprintf('%d\n',labs)];
                %fprintf(fidSeg,segstr);
%                 for slice=k:k+d-1
%                     %subMatSeg=matSeg(i:i+d-1,j:j+d-1,slice);
%                     subMatT1=matT1(i:i+d-1,j:j+d-1,slice);  
%                     subMatT2=matT2(i:i+d-1,j:j+d-1,slice);  
%                     %subMatSeg=uint16(subMatSeg);
%                     subMatT1=uint16(subMatT1);
%                     subMatT2=uint16(subMatT2);
%                     frameCnt=frameCnt+1;
%                     temp=sprintf('%06d.png',frameCnt);
%                     %imwrite(subMatSeg,[segPath,dirname,temp]);
%                     imwrite(subMatT1,[dataT1Path,dirname,temp]);
%                     imwrite(subMatT2,[dataT2Path,dirname,temp]);
%                 end
%                   for slice=1:dseg
%                     subMatSeg=tempmatseg(:,:,slice);
%                     subMatSeg=uint16(subMatSeg);
%                     segFrameCnt=segFrameCnt+1;
%                     temp=sprintf('%06d.png',segFrameCnt);
%                     imwrite(subMatSeg,[segPath,dirname,temp]);
%                 end

            end
        end
    end
%     
%     fclose(fidT1);
%     fclose(fidT2);
%     fclose(fidSeg);
return
