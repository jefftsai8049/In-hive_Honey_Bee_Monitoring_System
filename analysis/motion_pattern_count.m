close all
clc
clear all

file = fopen('bee_info_5_new/all/motion_pattern_count.csv');



days = 0;
IDs = 1;
while 1
    str = fgetl(file);
    if str == -1
        break
    end
    
    data = strsplit(str,',');
    if size(data) < 2
        
        days = days+1
        dateStr = cell2mat(data);
        dateStr = strrep(dateStr,'2015-','');
        dateStr = strrep(dateStr,'2016-','');
        date{days} = dateStr;
        IDs = 1;
    else
        motionPattern{days,IDs} = data;
        IDs = IDs+1;
    end
end

% groupAID = {'A';'B';'C'};
% groupBID = {'E';'F';'G'};
groupAID = {'A';'B';'C';'E';'F';'G'};
groupBID = {'H';'L';'K';'O';'P';'R'};

for i = 1:days
    IDs = 1;
    motionPatternCountA{i} = [0,0,0,0,0,0];
    motionPatternCountB{i} = [0,0,0,0,0,0];
    groupAIDs = 0;
    groupBIDs = 0;
    while size(motionPattern{i,IDs},1) ~= 0
        if sum(strcmp(groupAID,motionPattern{i,IDs}{1}(1))) > 0
            motionPatternCountA{i} = motionPatternCountA{i}+str2double(motionPattern{i,IDs}(2:end));
            groupAIDs = groupAIDs + 1;
        elseif sum(strcmp(groupBID,motionPattern{i,IDs}{1}(1))) > 0
            motionPatternCountB{i} = motionPatternCountA{i}+str2double(motionPattern{i,IDs}(2:end));
            groupBIDs = groupBIDs + 1;
        end
        
        IDs = IDs+1;
        if IDs > size(motionPattern,2)
            break;
        end
    end
%     motionPatternCountA{i} = motionPatternCountA{i}./groupAIDs;
%     motionPatternCountB{i} = motionPatternCountB{i}./groupBIDs;
end

x1 = [];
y1 = [];
for k = 1:days
    sumA = motionPatternCountA{k}(1)+motionPatternCountA{k}(2)+motionPatternCountA{k}(3)+motionPatternCountA{k}(4)+motionPatternCountA{k}(5)+motionPatternCountA{k}(6);
    sumB = motionPatternCountB{k}(1)+motionPatternCountB{k}(2)+motionPatternCountB{k}(3)+motionPatternCountB{k}(4)+motionPatternCountB{k}(5)+motionPatternCountB{k}(6);
    x1 = [x1,motionPatternCountA{k}(1)];
    y1 = [y1,motionPatternCountB{k}(1)];
end

x2 = [];
y2 = [];
for k = 1:days
    sumA = motionPatternCountA{k}(1)+motionPatternCountA{k}(2)+motionPatternCountA{k}(3)+motionPatternCountA{k}(4)+motionPatternCountA{k}(5)+motionPatternCountA{k}(6);
    sumB = motionPatternCountB{k}(1)+motionPatternCountB{k}(2)+motionPatternCountB{k}(3)+motionPatternCountB{k}(4)+motionPatternCountB{k}(5)+motionPatternCountB{k}(6);
    x2 = [x2,motionPatternCountA{k}(2)];
    y2 = [y2,motionPatternCountB{k}(2)];
    
end

x3 = [];
y3 = [];
for k = 1:days
    sumA = motionPatternCountA{k}(1)+motionPatternCountA{k}(2)+motionPatternCountA{k}(3)+motionPatternCountA{k}(4)+motionPatternCountA{k}(5)+motionPatternCountA{k}(6);
    sumB = motionPatternCountB{k}(1)+motionPatternCountB{k}(2)+motionPatternCountB{k}(3)+motionPatternCountB{k}(4)+motionPatternCountB{k}(5)+motionPatternCountB{k}(6);
    x3 = [x3,(motionPatternCountA{k}(3)+motionPatternCountA{k}(4)+motionPatternCountA{k}(5)+motionPatternCountA{k}(6))];
    y3 = [y3,(motionPatternCountB{k}(3)+motionPatternCountB{k}(4)+motionPatternCountB{k}(5)+motionPatternCountB{k}(6))];
end
aw = figure(1)
ap = gca;
bar([1:size(x1,2)],[x1;y1]')
% legend('Age D+7','Age D+0','Location','northwest');
legend('In-hive bee','Foraging bee','Location','northwest');
% xlabel('Date');
ylabel('Motion Pattern Count / Per Honeybee');
set(ap,'XTick',linspace(1,days,days));
set(ap,'XTickLabel',cell2mat(date'));
set(ap,'XTickLabelRotation',30);
set(aw, 'Position', [0 0 600 300]);

aw = figure(2)
ap = gca;
bar([1:size(x2,2)],[x2;y2]')
legend('In-hive bee','Foraging bee','Location','northwest');
% legend('Age D+7','Age D+0','Location','northwest');
% xlabel('Date');

set(ap,'XTick',linspace(1,days,days));
set(ap,'XTickLabel',cell2mat(date'));
set(ap,'XTickLabelRotation',30);

set(aw, 'Position', [0 0 600 300]);

aw = figure(3)
ap = gca;
bar([1:size(x3,2)],[x3;y3]')
legend('In-hive bee','Foraging bee','Location','northwest');
% legend('Age D+7','Age D+0','Location','northwest');
% xlabel('Date');
ylabel('Motion Pattern Count / Per Honeybee');
set(ap,'XTick',linspace(1,days,days));
set(ap,'XTickLabel',cell2mat(date'));
set(ap,'XTickLabelRotation',30);
set(aw, 'Position', [0 0 600 300]);