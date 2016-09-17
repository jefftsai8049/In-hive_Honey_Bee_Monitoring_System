fileName = '../out/bee_info/individual_info.csv';

isBreak = 0;
[labelY,static,loitering,moving1,moving2,moving3,moving4] = importfile6D(fileName,1,inf);
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
moving = moving1+moving2+moving3+moving4;


scatter3(static(1:breakPoint,:),loitering(1:breakPoint,:),moving(1:breakPoint,:),'ob');
% text(static+0.01,loitering,labelY);
% % title('Motion Pattern in 2D PCA ');
hold on
p = scatter3(static(breakPoint+1:end,:),loitering(breakPoint+1:end,:),moving(breakPoint+1:end,:),'xr');
% hold off
% minX = floor(min(static));
% maxX = ceil(max(static));
% minY = min(loitering);
% maxY = max(loitering);
% yTick = linspace(minY,maxY,10);
% set(gca,'YTick' , yTick)
% set(gca,'YTickLabel', num2str(yTick' , '%0.2f'))
% labelX = linspace(minX,maxX,11);
% set(gca,'XTick' , labelX)
% set(gca,'XTickLabel', num2str(labelX' , '%0.2f'))
% 
% % legend('Field bee','In-hive bee');
% legend('Age D+8','Age D+0');
xlabel('Static');
ylabel('Loitering');
zlabel('Moving');
% 
% saveas(p,[dataSet,'/out/',day,'.png']);