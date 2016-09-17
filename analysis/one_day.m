cd bee_info_1
cd days

dirInfo = dir;
dirInfo = dirInfo(3:end);
cd ../..
for i = 1:size(dirInfo)
%     clear;
    fileName = ['bee_info_1/days/',dirInfo(i).name,'/individual_PCA.csv'];
    clear pc1 pc2 label j
    [label,pc1,pc2] = importfile(fileName,1,inf);

    for j = 1 : size(label)
        if label{j,1}(1) == 'E'||label{j,1}(1) =='F'||label{j,1}(1) =='G'
            breakPoint = j;
            break
        end
    end
    figure(i);
    
    plot(pc1(1:breakPoint-1,:),pc2(1:breakPoint-1,:),'ob');
    text(pc1+0.01,pc2,label);
    hold on
    plot(pc1(breakPoint:end,:),pc2(breakPoint:end,:),'xr');
%     title(strcat(strcat('Motion Pattern in 2D PCA (',dirInfo(i).name),')'));
    minX = floor(min(pc1));
    maxX = ceil(max(pc1));
    minY = min(pc2);
    maxY = max(pc2);    
    labelY = linspace(minY,maxY,10);
    set(gca,'YTick' , labelY)
    set(gca,'YTickLabel', num2str(labelY' , '%0.2f'))
    labelX = linspace(minX,maxX,11);
    set(gca,'XTick' , labelX)
    set(gca,'XTickLabel', num2str(labelX' , '%0.2f'))
    xlabel('PC1');
    ylabel('PC2')
end