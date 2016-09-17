clear all
clc
close all

file = fopen('bee_info_0_new/all/trajectory_info.csv');

% groupAID = {'A';'B';'C';'E';'F';'G'};
groupAID = {'A';'B';'C'};
% groupBID = {'H';'L';'K';'O';'P';'R'};
groupBID = {'E','G','H'};
dayList = {};

distanceA = {};
distanceB = {};

velocityA = {};
velocityB = {};

IDsAG = {};
IDsBG = {};

trajectoryA = {};
trajectoryB = {};

str = fgetl(file);
date = {};

while 1
    str = fgetl(file);
    if str == -1
        break;
    end
    
    data = strsplit(str,',');
    
    ID = data{1};
    timeStr = strsplit(data{2},'-');
    year = timeStr{1};
    month = timeStr{2};
    day = timeStr{3};
    hour = timeStr{4};
    minute = timeStr{5};
    second = timeStr{6};
    
    distance = str2double(cell2mat(data(3)));
    velocity = str2double(cell2mat(data(4)));
    
    
    inList = 0;
    
    for i = 1:size(dayList,2)
        if dayList{i} == day
            inList = 1;
            dayID = i;
            break;
        end
    end
    
    if inList == 0
        dayList{size(dayList,2)+1} = day
        dayID = size(dayList,2);
        
        date{dayID} = [month,'-',day];
        
        distanceA{dayID} = 0;
        velocityA{dayID} = 0;
        IDsAG{dayID} = 0;
        trajectoryA{dayID} = 0;
        
        distanceB{dayID} = 0;
        velocityB{dayID} = 0;
        IDsBG{dayID} = 0;
        trajectoryB{dayID} = 0;
        inList = 1;
        
    end
    
    if sum(strcmp(groupAID,ID(1))) > 0
        distanceA{dayID} = distanceA{dayID}+distance;
        velocityA{dayID} = velocityA{dayID}+velocity;
        trajectoryA{dayID} = trajectoryA{dayID}+1;
    elseif sum(strcmp(groupBID,ID(1))) > 0
        distanceB{dayID} = distanceB{dayID}+distance;
        velocityB{dayID} = velocityB{dayID}+velocity;
        trajectoryB{dayID} = trajectoryB{dayID}+1;
    end
end
%%
file = fopen('bee_info_0_new/all/dailyInfo.csv');

group = 1;
while 1
    dateStr = fgetl(file);
    if dateStr == -1
        break
    end
    IDStr = fgetl(file);
    countStr = fgetl(file);
    
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
        % % %          || info.ID{i}{j}(1) == 'E' || info.ID{i}{j}(1) == 'F' || info.ID{i}{j}(1) == 'G'
        if info.ID{i}{j}(1) == 'A' || info.ID{i}{j}(1) == 'B' || info.ID{i}{j}(1) == 'C'
            %         if info.ID{i}{j}(1) == 'A' || info.ID{i}{j}(1) == 'B' || info.ID{i}{j}(1) == 'C' || info.ID{i}{j}(1) == 'E' || info.ID{i}{j}(1) == 'F' || info.ID{i}{j}(1) == 'G'
            info.G1Sum{i} = info.G1Sum{i}+info.count{i}(j);
            info.G1TagSum{i} = info.G1TagSum{i}+1;
        else
            info.G2Sum{i} = info.G2Sum{i}+info.count{i}(j);
            info.G2TagSum{i} = info.G2TagSum{i}+1;
            
        end
    end
    
    
end


%%
xDis= cell2mat(distanceA)./cell2mat(info.G1TagSum);
yDis = cell2mat(distanceB)./cell2mat(info.G2TagSum);
xTra = cell2mat(distanceA)./cell2mat(info.G1TagSum);
yTra = cell2mat(distanceB)./cell2mat(info.G2TagSum);
xV = cell2mat(velocityA)./cell2mat(info.G1TagSum);
yV = cell2mat(velocityB)./cell2mat(info.G2TagSum);

% xDis= cell2mat(distanceA);
% yDis = cell2mat(distanceB);
% xTra = cell2mat(distanceA)./cell2mat(info.G1TagSum);
% yTra = cell2mat(distanceB)./cell2mat(info.G2TagSum);
% xV = cell2mat(velocityA)./cell2mat(info.G1TagSum);
% yV = cell2mat(velocityB)./cell2mat(info.G2TagSum);



ratio = 0.01125;
xDis = xDis.*ratio;
yDis = yDis.*ratio;
xTra = xTra.*ratio;
yTra = yTra.*ratio;
xV = xV.*ratio;
yV = yV.*ratio;

%%
aw = figure(1);
% aw = subplot(3,1,1);
ap = gca;
% set(aw, 'Position', [0 0 600 300]);
bar([1:size(distanceA,2)],[xDis;yDis]')
ylabel('Distance (cm)');
% legend('Age D+7','Age D+0');
legend('Field bee','In-hive bee');
set(ap,'XTick',linspace(1,days,days));
set(ap,'XTickLabel',date);
set(ap,'XTickLabelRotation',30);

figure(2);
bw = subplot(2,1,1);
bp = gca;
% set(bw, 'Position', [0 0 600 300]);
bar([1:size(velocityA,2)],[xTra;yTra]');
ylabel('Moving distance (cm)');
% legend('Age D+7','Age D+0');
% legend('Field bee','In-hive bee');
legend('Freezingg Method','Vacuum Method')
set(bp,'XTick',linspace(1,days,days));
set(bp,'XTickLabel',date);
set(bp,'XTickLabelRotation',30);

% cw = figure(3);
cw = subplot(2,1,2);
cp = gca;

% set(cw, 'Position', [0 0 600 300]);
bar([1:size(velocityA,2)],[xV;yV]');
ylabel('Moving speed (cm/sec)');
% set(gca,'YLim',[0 0.7],'YTick',0:0.1:0.7);
% legend('Age D+7','Age D+0');
% legend('Field bee','In-hive bee');
legend('Freezingg Method','Vacuum Method')
set(cp,'XTick',linspace(1,days,days));
set(cp,'XTickLabel',date);
set(cp,'XTickLabelRotation',30);