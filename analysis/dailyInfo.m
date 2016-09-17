file = fopen(fileName);

group = 1;
while 1
    dateStr = fgetl(file);
    if dateStr == -1
        break
    end
    IDStr = fgetl(file);
    countStr = fgetl(file);
    %     if group == 1
    %         dateStr = fgetl(file);
    %       IDStr = fgetl(file);
    %         countStr = fgetl(file);
    %     end
    
    info.date{group} = dateStr;
    info.ID{group} = strsplit(IDStr,',');
    countCell = strsplit(countStr,',');
    %   count = zeros(1,size(countCell,2));
    for i = 1:size(countCell,2)
        count(1,i) = str2num(countCell{1,i});
    end
    
    info.count{group} = count;
    
    group = group + 1;
    clear count
end
fclose(file);
clear count dateStr countStr countCell i str IDSte dateStr IDStr
clear file group

days = size(info.date,2);

for i = 1:size(info.date,2)
    info.sum{i} = sum(info.count{i});
    info.IDSize{i} = size(info.ID{i},2);
    
    info.G1Sum{i} = 0;
    info.G1TagSum{i} = 0;
    info.G2Sum{i} = 0;
    info.G2TagSum{i} = 0;
    for j = 1:size(info.count{i},2)
%                if info.ID{i}{j}(1) == 'E' || info.ID{i}{j}(1) == 'G' || info.ID{i}{j}(1) == 'H'
%         if info.ID{i}{j}(1) == 'A' || info.ID{i}{j}(1) == 'B' || info.ID{i}{j}(1) == 'C'
                    if info.ID{i}{j}(1) == 'A' || info.ID{i}{j}(1) == 'B' || info.ID{i}{j}(1) == 'C'|| info.ID{i}{j}(1) == 'E' || info.ID{i}{j}(1) == 'F' || info.ID{i}{j}(1) == 'G'
            info.G1Sum{i} = info.G1Sum{i}+info.count{i}(j);
            info.G1TagSum{i} = info.G1TagSum{i}+1;
        else
            info.G2Sum{i} = info.G2Sum{i}+info.count{i}(j);
            info.G2TagSum{i} = info.G2TagSum{i}+1;
            
        end
    end
    
    
end

%%
stackedbar = @(x, A) bar(x, A);
prettyline = @(x, y) plot(x, y, 'r-o', 'LineWidth', 2);

ax = gca;
% ax.XTickLabelRotation=30;
% set(ax,'XLim',[0 days],'XTick',1:days,'XTickLabel',info.date)
% title(titleName);
y1 = cell2mat(info.sum);
y2 = cell2mat(info.IDSize);

[px,py1,py2] = plotyy(linspace(1,days,days),y1(:,1:days),linspace(1,days,days),y2(:,1:days),stackedbar, prettyline);
% [px,py1,py2] = plotyy(linspace(1,days-1,days-1),y1(:,2:days),linspace(1,days-1,days-1),y2(:,2:days),stackedbar, prettyline);
ax.XTickLabelRotation=30;
set(px,'XLim',[0 days+1],'XTick',1:days,'XTickLabel',info.date);
% set(px,'XLim',[0 days],'XTick',1:days-1,'XTickLabel',info.date(2:end));
xlabel('Date');
ylabel(px(1),'Effectively Trajectory')
ylabel(px(2),'Detected Honeybee')
% set(gcb,'XLim',[0 days+1]);

aw = figure(2);
set(aw, 'Position', [0 0 600 500]);
ax = gca;
% info.G2Sum = mat2cell(zeros(size(info.G1Sum)),1);
% info.G2TagSum = mat2cell(zeros(size(info.G1TagSum)),1);
[bx,by1,by2] = plotyy(linspace(1,days,days),[cell2mat(info.G1Sum);cell2mat(info.G2Sum)]',linspace(1,days,days),cell2mat(info.G1TagSum),stackedbar, prettyline);
% bar(linspace(1,16,16),[cell2mat(info.G1Sum);cell2mat(info.G2Sum)]')
% hold on
for i = 1:days
    if info.G2TagSum{i} ~= 0
        startPoint = i;
        break;
    end
end

set(bx,'NextPlot','add')
plot(bx(2),linspace(startPoint,days,days-startPoint+1),cell2mat(info.G2TagSum(startPoint:days)),'bx-', 'LineWidth', 2);
% legend('Freezingg Method Trajectory Numbers','Vacuum Method Trajectory Numbers','Freezingg Method Honeybee Numbers','Vacuum Method Honeybee Numbers')
legend([controlLegendName,' Trajectory Numbers'],[experimentLegendName,' Trajectory Numbers'],[controlLegendName,' Detected Honeybee Numbers'],[experimentLegendName,' Detected Honeybee Numbers'])
% legend('Field Bee Trajectory Numbers','In-hive Bee Trajectory Numbers','Field Bee Detected Honeybee Numbers','In-hive Bee Detected Honeybee Numbers')
ax.XTickLabelRotation=30;
set(bx(1),'YLim',[0 3000],'YTick',0:500:3000);
set(bx(2),'YLim',[0 50],'YTick',0:10:50);
ylabel(bx(1),'Effectively Trajectory')
ylabel(bx(2),'Detected Honeybee')

set(bx,'XLim',[0 days+1],'XTick',1:days,'XTickLabel',info.date);

ratio = y1./y2;
