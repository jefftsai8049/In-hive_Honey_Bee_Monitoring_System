clear all
clc
close all

file = fopen('bee_info_5_new/all/trajectory_info.csv');
longDistanceThreshold = 1000;
highSpeedThreshold = 40;
slowSpeedThreshold = 30;
highDetectedTimeThreshold = 50;
G2Start = 1;
div = 12;


title = fgetl(file);

dateTimeList = {};

trajectoryCountG1 = {};
trajectoryCountG2 = {};
detectedTimeG1 = {};
detectedTimeG2 = {};
distanceG1 = {};
velocityG2 = {};

IDListG1 = {};
IDListG2 = {};


longDistanceTrajectoryCountG1 = {};
longDistanceTrajectoryCountG2 = {};
highSpeedTrajectoryCountG1 = {};
highSpeedTrajectoryCountG2 = {};
highDetectedTimeTrajectoryCountG1 = {};
highDetectedTimeTrajectoryCountG2 = {};
slowLongTrajectoryG1 = {};
slowLongTrajectoryG2 = {};

staticRatioG1 = {};
staticRatioG2 = {};
loiteringRatioG1 = {};
loiteringRatioG2 = {};
movingRatioG1 = {};
movingRatioG2 = {};
staticG1 = {};
staticG2 = {};
loiteringG1 = {};
loiteringG2 = {};
movingG1 = {};
movingG2 = {};



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
    
    
    dateTime = [month,'/',day,'-',hour];
    distance = str2num(data{3});
    velocity = str2num(data{4});
    detectedTime = str2num(data{5});
    group = str2num(data{16})+1;
    
    staticRatio = str2num(data{13});
    loiteringRatio = str2num(data{14});
    movingRatio = str2num(data{15});
    
    
    inList = 0;
    dateTimeID = 1;
    for i = 1:size(dateTimeList,2)
        if dateTime == dateTimeList{i}
            inList = 1;
            dateTimeID = i;
            break;
        end
    end
    
    if inList == 0
        newID = size(dateTimeList,2)+1;
        dateTimeList{newID} = dateTime;
        trajectoryCountG1{newID} = 0;
        trajectoryCountG2{newID} = 0;
        distanceG1{newID} = 0;
        distanceG2{newID} = 0;
        velocityG1{newID} = 0;
        velocityG2{newID} = 0;
        
        IDListG1{newID} = [];
        IDListG2{newID} = [];
        longDistanceTrajectoryCountG1{newID} = 0;
        longDistanceTrajectoryCountG2{newID} = 0;
        highSpeedTrajectoryCountG1{newID} = 0;
        highSpeedTrajectoryCountG2{newID} = 0;
        highDetectedTimeTrajectoryCountG1{newID} = 0;
        highDetectedTimeTrajectoryCountG2{newID} = 0;
        detectedTimeG1{newID} = 0;
        detectedTimeG2{newID} = 0;
        slowLongTrajectoryG1{newID} = 0;
        slowLongTrajectoryG2{newID} = 0;
        
        
        staticRatioG1{newID} = 0;
        staticRatioG2{newID} = 0;
        loiteringRatioG1{newID} = 0;
        loiteringRatioG2{newID} = 0;
        movingRatioG1{newID} = 0;
        movingRatioG2{newID} = 0;
        staticG1{newID} = [];
        staticG2{newID} = [];
        loiteringG1{newID} = [];
        loiteringG2{newID} = [];
        movingG1{newID} = [];
        movingG2{newID} = [];
        
        
        inList = 1;
        dateTimeID = newID;
        dateTime
    end
    
    
    
    if group == 1
        trajectoryCountG1{dateTimeID} = trajectoryCountG1{dateTimeID} + 1;
        detectedTimeG1{newID} = detectedTimeG1{newID} + detectedTime;
        
        if distance > longDistanceThreshold
            longDistanceTrajectoryCountG1{dateTimeID} = longDistanceTrajectoryCountG1{dateTimeID} + 1;
        end
        if velocity > highSpeedThreshold
            highSpeedTrajectoryCountG1{dateTimeID} = highSpeedTrajectoryCountG1{dateTimeID} + 1;
        end
        if detectedTime > highDetectedTimeThreshold
            highDetectedTimeTrajectoryCountG1{dateTimeID} = highDetectedTimeTrajectoryCountG1{dateTimeID} + 1;
        end
        
        if velocity < slowSpeedThreshold && distance > longDistanceThreshold
            slowLongTrajectoryG1{dateTimeID} = slowLongTrajectoryG1{dateTimeID} + 1;
        end
        distanceG1{newID} = distanceG1{newID}+distance;
        velocityG1{newID} = velocityG1{newID}+velocity;
        
        staticRatioG1{newID} = staticRatioG1{newID}+staticRatio;
        loiteringRatioG1{newID} = loiteringRatioG1{newID}+loiteringRatio;
        movingRatioG1{newID} = movingRatioG1{newID}+movingRatio;
        
        staticG1{newID} = [staticG1{newID},staticRatio];
        loiteringG1{newID} = [loiteringG1{newID},loiteringRatio];
        movingG1{newID} = [movingG1{newID},movingRatio];
        
        inIDList = 0;
        
        for m = 1:size(IDListG1{newID},1)
            if ID == IDListG1{newID}(m,:)
                inIDList = 1;
                break;
            end
        end
        
        if inIDList == 0 || size(IDListG1{newID},1) == 0
            IDListG1{newID} = [IDListG1{newID};ID];
        end
        
    elseif group == 2
        detectedTimeG2{newID} = detectedTimeG2{newID} + detectedTime;
        trajectoryCountG2{dateTimeID} = trajectoryCountG2{dateTimeID} + 1;
        if distance > longDistanceThreshold
            longDistanceTrajectoryCountG2{dateTimeID} = longDistanceTrajectoryCountG2{dateTimeID} + 1;
        end
        if velocity > highSpeedThreshold
            highSpeedTrajectoryCountG2{dateTimeID} = highSpeedTrajectoryCountG2{dateTimeID} + 1;
        end
        if detectedTime > highDetectedTimeThreshold
            highDetectedTimeTrajectoryCountG2{dateTimeID} = highDetectedTimeTrajectoryCountG2{dateTimeID} + 1;
        end
        if velocity < slowSpeedThreshold && distance > longDistanceThreshold
            slowLongTrajectoryG2{dateTimeID} = slowLongTrajectoryG2{dateTimeID} + 1;
        end
        staticRatioG2{newID} = staticRatioG2{newID}+staticRatio;
        loiteringRatioG2{newID} = loiteringRatioG2{newID}+loiteringRatio;
        movingRatioG2{newID} = movingRatioG2{newID}+movingRatio;
        distanceG2{newID} = distanceG2{newID}+distance;
        velocityG2{newID} = velocityG2{newID}+velocity;
        
        staticG2{newID} = [staticG2{newID},staticRatio];
        loiteringG2{newID} = [loiteringG2{newID},loiteringRatio];
        movingG2{newID} = [movingG2{newID},movingRatio];
        
        inIDList = 0;
        for m = 1:size(IDListG2{newID},1)
            if ID == IDListG2{newID}(m,:)
                inIDList = 1;
                break;
            end
        end
        
        if inIDList == 0|| size(IDListG2{newID},1) == 0
            IDListG2{newID} = [IDListG2{newID};ID];
        end
        
    end
end



%%
close all

for i = 1:size(longDistanceTrajectoryCountG1,2)
    G2Start = i;
    if longDistanceTrajectoryCountG2{i}~=0
        break;
    end
end
IDListSizeG1 = [];
IDListSizeG2 = [];
for i = 1:size(IDListG1,2)
    IDListSizeG1 = [IDListSizeG1, size(IDListG1{i},1)];
    IDListSizeG2 = [IDListSizeG2, size(IDListG2{i},1)];
end
xCount = linspace(1,size(dateTimeList,2),size(dateTimeList,2));
xTick = linspace(1,size(dateTimeList,2),size(dateTimeList,2)/div);
xLabel = cell2mat(dateTimeList');
xLabel = xLabel(floor(xTick),:);
% plot long distance trajectory bar plot
longDistanceRatioA = cell2mat(longDistanceTrajectoryCountG1)./cell2mat(trajectoryCountG1);
longDistanceRatioB = cell2mat(longDistanceTrajectoryCountG2)./cell2mat(trajectoryCountG2);
aw = figure(1);
ab = bar([1:size(longDistanceRatioA,2)],[longDistanceRatioA;longDistanceRatioB]');
ab(1).FaceColor = 'b';
ab(2).FaceColor = 'r';
ylabel('Long Distance Trajectory Ratio');
hold on
m = polyfit(xCount,longDistanceRatioA,1);
plot(xCount,xCount.*m(1)+m(2),'b','LineWidth', 2);
hold on
m = polyfit(xCount(G2Start:end),longDistanceRatioB(G2Start:end),1);
plot(xCount(G2Start:end),xCount(G2Start:end).*m(1)+m(2),'r','LineWidth', 2);
legend('Age D+8','Age D+0')
set(gca,'XTick',xTick);
set(gca,'XTickLabel',xLabel);
set(gca,'XTickLabelRotation',30);
set(aw, 'Position', [0 0 600 300]);


% plot high speed trajectory bar plot
highSpeedRatioA = cell2mat(highSpeedTrajectoryCountG1)./cell2mat(trajectoryCountG1);
highSpeedRatioB = cell2mat(highSpeedTrajectoryCountG2)./cell2mat(trajectoryCountG2);
aw = figure(2);
ab = bar([1:size(highSpeedRatioA,2)],[highSpeedRatioA;highSpeedRatioB]');
ab(1).FaceColor = 'b';
ab(2).FaceColor = 'r';
ylabel('High Speed Trajectory Ratio');
hold on
m = polyfit(xCount,highSpeedRatioA,1);
plot(xCount,xCount.*m(1)+m(2),'b','LineWidth', 2);
hold on
m = polyfit(xCount(G2Start:end),highSpeedRatioB(G2Start:end),1);
plot(xCount(G2Start:end),xCount(G2Start:end).*m(1)+m(2),'r','LineWidth', 2);
legend('Age D+8','Age D+0')
set(gca,'XTick',xTick);
set(gca,'XTickLabel',xLabel);
set(gca,'XTickLabelRotation',30);
set(aw, 'Position', [0 0 600 300]);

% plot high detected time trajectory bar plot
highDetectedTimeRatioA = cell2mat(highDetectedTimeTrajectoryCountG1)./cell2mat(trajectoryCountG1);
highDetectedTimeRatioB = cell2mat(highDetectedTimeTrajectoryCountG2)./cell2mat(trajectoryCountG2);
aw = figure(3);
ab = bar([1:size(highDetectedTimeRatioA,2)],[highDetectedTimeRatioA;highDetectedTimeRatioB]');
ab(1).FaceColor = 'b';
ab(2).FaceColor = 'r';
ylabel('High Detected Time Trajectory Ratio');
hold on
m = polyfit(xCount,highDetectedTimeRatioA,1);
plot(xCount,xCount.*m(1)+m(2),'b','LineWidth', 2);
hold on
m = polyfit(xCount(G2Start:end),highDetectedTimeRatioB(G2Start:end),1);
plot(xCount(G2Start:end),xCount(G2Start:end).*m(1)+m(2),'r','LineWidth', 2);
legend('Age D+8','Age D+0')
set(gca,'XTick',xTick);
set(gca,'XTickLabel',xLabel);
set(gca,'XTickLabelRotation',30);
set(aw, 'Position', [0 0 600 300]);

staticVarG1 = [];
staticVarG2 = [];
for i = 1:size(staticG1,2)
    staticVarG1 = [staticVarG1,var(staticG1{i})];
    staticVarG2 = [staticVarG2,var(staticG2{i})];
end

% static ratio
staticRatioA = cell2mat(staticRatioG1)./cell2mat(trajectoryCountG1);
staticRatioB = cell2mat(staticRatioG2)./cell2mat(trajectoryCountG2);
aw = figure(4);
% errorbar(staticRatioA,staticVarG1);
% hold on
% errorbar(staticRatioB,staticVarG2);
ab = bar([1:size(staticRatioA,2)],[staticRatioA;staticRatioB]');
ab(1).FaceColor = 'b';
ab(2).FaceColor = 'r';
ylabel('Static Pattern Ratio');
hold on
m = polyfit(xCount,staticRatioA,1);
plot(xCount,xCount.*m(1)+m(2),'b','LineWidth', 2);
hold on
m = polyfit(xCount(G2Start:end),staticRatioB(G2Start:end),1);
plot(xCount(G2Start:end),xCount(G2Start:end).*m(1)+m(2),'r','LineWidth', 2);
legend('Age D+8','Age D+0')
% set(gca,'YLim',[0.1 0.4],'YTick',0.1:0.1:0.4);
set(gca,'XTick',xTick);
set(gca,'XTickLabel',xLabel);
set(gca,'XTickLabelRotation',30);
set(aw, 'Position', [0 0 600 300]);

% loitering ratio
loiteringRatioA = cell2mat(loiteringRatioG1)./cell2mat(trajectoryCountG1);
loiteringRatioB = cell2mat(loiteringRatioG2)./cell2mat(trajectoryCountG2);
aw = figure(5);
ab = bar([1:size(loiteringRatioA,2)],[loiteringRatioA;loiteringRatioB]');
ab(1).FaceColor = 'b';
ab(2).FaceColor = 'r';
ylabel('Loitering Pattern Ratio');
hold on
m = polyfit(xCount,loiteringRatioA,1);
plot(xCount,xCount.*m(1)+m(2),'b','LineWidth', 2);
hold on
m = polyfit(xCount(G2Start:end),loiteringRatioB(G2Start:end),1);
plot(xCount(G2Start:end),xCount(G2Start:end).*m(1)+m(2),'r','LineWidth', 2);
legend('Age D+8','Age D+0')
set(gca,'YLim',[0.1 0.4],'YTick',0.1:0.1:0.4);
set(gca,'XTick',xTick);
set(gca,'XTickLabel',xLabel);
set(gca,'XTickLabelRotation',30);
set(aw, 'Position', [0 0 600 300]);
set(gca,'XTickLabelRotation',30);
set(aw, 'Position', [0 0 600 300]);

% moving ratio
movingRatioA = cell2mat(movingRatioG1)./cell2mat(trajectoryCountG1);
movingRatioB = cell2mat(movingRatioG2)./cell2mat(trajectoryCountG2);
aw = figure(6);
ab = bar([1:size(movingRatioA,2)],[movingRatioA;movingRatioB]');
ab(1).FaceColor = 'b';
ab(2).FaceColor = 'r';
ylabel('Moving Pattern Ratio');
hold on
m = polyfit(xCount,movingRatioA,1);
plot(xCount,xCount.*m(1)+m(2),'b','LineWidth', 2);
hold on
m = polyfit(xCount(G2Start:end),movingRatioB(G2Start:end),1);
plot(xCount(G2Start:end),xCount(G2Start:end).*m(1)+m(2),'r','LineWidth', 2);
legend('Age D+8','Age D+0')
set(gca,'YLim',[0.3 0.8],'YTick',0.3:0.1:0.8);
set(gca,'XTick',xTick);
set(gca,'XTickLabel',xLabel);
set(gca,'XTickLabelRotation',30);
set(aw, 'Position', [0 0 600 300]);

% slow long trajectory
movingRatioA = cell2mat(slowLongTrajectoryG1)./cell2mat(trajectoryCountG1);
movingRatioB = cell2mat(slowLongTrajectoryG2)./cell2mat(trajectoryCountG2);
aw = figure(7);
ab = bar([1:size(movingRatioA,2)],[movingRatioA;movingRatioB]');
ab(1).FaceColor = 'b';
ab(2).FaceColor = 'r';
ylabel('Slow and Long Trajectory Ratio');
hold on
m = polyfit(xCount,movingRatioA,1);
plot(xCount,xCount.*m(1)+m(2),'b','LineWidth', 2);
hold on
m = polyfit(xCount(G2Start:end),movingRatioB(G2Start:end),1);
plot(xCount(G2Start:end),xCount(G2Start:end).*m(1)+m(2),'r','LineWidth', 2);
legend('Age D+8','Age D+0')
set(gca,'XTick',xTick);
set(gca,'XTickLabel',xLabel);
set(gca,'XTickLabelRotation',30);
set(aw, 'Position', [0 0 600 300]);

% plot high detected time trajectory bar plot
detectedTimeA = cell2mat(detectedTimeG1)./IDListSizeG1;
detectedTimeB = cell2mat(detectedTimeG2)./IDListSizeG2;
aw = figure(8);
ab = bar([1:size(detectedTimeA,2)],[detectedTimeA;detectedTimeB]');
ab(1).FaceColor = 'b';
ab(2).FaceColor = 'r';
ylabel('Detected Time (sec)');
hold on
m = polyfit(xCount,detectedTimeA,1);
plot(xCount,xCount.*m(1)+m(2),'b','LineWidth', 2);
hold on
m = polyfit(xCount(G2Start:end),detectedTimeB(G2Start:end),1);
plot(xCount(G2Start:end),xCount(G2Start:end).*m(1)+m(2),'r','LineWidth', 2);
legend('Age D+8','Age D+0')
set(gca,'XTick',xTick);
set(gca,'XTickLabel',xLabel);
set(gca,'XTickLabelRotation',30);
set(aw, 'Position', [0 0 600 300]);

% plot distance bar plot
detectedTimeA = cell2mat(distanceG1)./IDListSizeG1;
detectedTimeB = cell2mat(distanceG2)./IDListSizeG2;
aw = figure(9);
ab = bar([1:size(detectedTimeA,2)],[detectedTimeA;detectedTimeB]');
ab(1).FaceColor = 'b';
ab(2).FaceColor = 'r';
ylabel('Distance (pixels)');
hold on
m = polyfit(xCount,detectedTimeA,1);
plot(xCount,xCount.*m(1)+m(2),'b','LineWidth', 2);
hold on
m = polyfit(xCount(G2Start:end),detectedTimeB(G2Start:end),1);
plot(xCount(G2Start:end),xCount(G2Start:end).*m(1)+m(2),'r','LineWidth', 2);
legend('Age D+8','Age D+0')
set(gca,'XTick',xTick);
set(gca,'XTickLabel',xLabel);
set(gca,'XTickLabelRotation',30);
set(aw, 'Position', [0 0 600 300]);

% plot velocity bar plot
detectedTimeA = cell2mat(velocityG1)./IDListSizeG1;
detectedTimeB = cell2mat(velocityG2)./IDListSizeG2;
aw = figure(10);
ab = bar([1:size(detectedTimeA,2)],[detectedTimeA;detectedTimeB]');
ab(1).FaceColor = 'b';
ab(2).FaceColor = 'r';
ylabel('Velocity (pixels)');
hold on
m = polyfit(xCount,detectedTimeA,1);
plot(xCount,xCount.*m(1)+m(2),'b','LineWidth', 2);
hold on
m = polyfit(xCount(G2Start:end),detectedTimeB(G2Start:end),1);
plot(xCount(G2Start:end),xCount(G2Start:end).*m(1)+m(2),'r','LineWidth', 2);
legend('Age D+8','Age D+0')
set(gca,'XTick',xTick);
set(gca,'XTickLabel',xLabel);
set(gca,'XTickLabelRotation',30);
set(aw, 'Position', [0 0 600 300]);