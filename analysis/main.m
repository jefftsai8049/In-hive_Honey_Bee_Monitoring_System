
isBreak = 0;
[labelY,pc1,pc2] = importfile(fileName,1,inf);
for j = 1 : size(labelY)
%     if labelY{j,1}(1) == 'E'||labelY{j,1}(1) =='F'||labelY{j,1}(1) =='G'
            if labelY{j,1}(1) == 'H'||labelY{j,1}(1) =='L'||labelY{j,1}(1) =='K'||labelY{j,1}(1) =='O'||labelY{j,1}(1) =='P'||labelY{j,1}(1) =='R'
        breakPoint = j;
        isBreak = 1;
        break
    end
end

if isBreak == 0
   breakPoint = size(labelY,1);
end

scatter(pc1(1:breakPoint,:),pc2(1:breakPoint,:),'ob');
text(pc1+0.01,pc2,labelY);
% title('Motion Pattern in 2D PCA ');
hold on
p = scatter(pc1(breakPoint+1:end,:),pc2(breakPoint+1:end,:),'xr');
hold off
minX = floor(min(pc1));
maxX = ceil(max(pc1));
minY = min(pc2);
maxY = max(pc2);
yTick = linspace(minY,maxY,10);
set(gca,'YTick' , yTick)
set(gca,'YTickLabel', num2str(yTick' , '%0.2f'))
labelX = linspace(minX,maxX,11);
set(gca,'XTick' , labelX)
set(gca,'XTickLabel', num2str(labelX' , '%0.2f'))

% legend('Field bee','In-hive bee');
legend('Age D+8','Age D+0');
xlabel('PC1');
ylabel('PC2')

saveas(p,[dataSet,'/out/',day,'.png']);