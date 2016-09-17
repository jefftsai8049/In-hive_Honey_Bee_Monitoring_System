clear all
close all
clc

% subG1 = {'AF';'AO';'AR';'BA';'BB';'BG';'BK';'BL';'BO';'BP';'BR';'BS';'BU';'BY';'BZ';'CA';'CG';'CH';'CK';'CP';'EA';'FC';'FE';'FF';'FL';'FT';'FZ';'GL';'GP';};
subG1 = {'AA';'AC';'AG';'AL';'AP';'AR';'AT';'AV';'BB';'BF';'BH';'BO';'BP';'BP';'BR';'BT';'BY';'BZ';'CF';'CH';'CR';'CP';'CT';'CG';'CZ';'EB';'EG';'DA';'FB';'FE';'FF';'FC';'FG';'FH';'FL';'FK';'FR';'FT';'FY';'GA';'GB';'GC';'GF';'GK';'GP';'GO';'GT';'GZ'};
% subG1 = {'AR';'BB';'BO';'BP';'BR';'BY';'BZ';'CH';'CP';'FE';'FF';'FL';'GP'};
% subG1 = {'AF';'AO';'BA';'BG';'BK';'BL';'BS';'BU';'CA';'CG';'CK';'CP';'EA';'FC';'FT';'FZ';'GL';};
% subG1 = {'AF';'BG';'BP';'BO';'AO';'BB'};
% subG1 = {'AO'};

% subG2 = {'HB';'HE';'HL';'HO';'HS';'HZ';'KG';'KK';'KZ';'LH';'LL';'LO';'LP';'LS';'LY';'OA';'OB';'OH';'OO';'OY';'OZ';'PB';'PC';'PL';'PZ';'RA';'RB';'RE';'RF';'RK';'RO';'RR';'RT';'RU';};
% subG2 = {'OO';'OZ';'HL';'HE';'LY';'RO';'RB';'OB';'RS';'RU';'RE';'RF';'RU';'RS';'KK';'HS';'OH';'PL';'OA';'OP';'HA';'OS';'HA'};
% subG2 = {'OO';'HL';'HE';'RO';'RS';'BO';'RB';'RU';'HS';'RF';'PL';'OA';'OS';'HA'};
% subG2 = {'HT';'HZ';'HF';'HS';'HA';'HL';'KF';'KT';'KS';'KU';'KP';'KL';'KK';'LC';'LA';'LB';'LC';'LZ';'LO';'LK';'LG';'LU';'OB';'OC';'OH';'OF';'OK';'OS';'OA';'OG';'OZ';'OO';'PA';'PG';'PC';'PR';'PO';'PZ';'PL';'PU';'PF';'PK';'PB';'PT';'RS';'RB';'RF';'RR';'RH';'RZ';'RC';'RY';'RU';'RP';'RG'};
% subG2 = {'HL';'OO';'RS';'RB';'RF';'OS';'HA'};
subG2 = {'HL'};
% subG2 = {'OO';'OZ';'HL';'RB';'OB';'RS';'RU';'RF';'KK';'HS';'OH';'PL';'OA';'OS';'HA'};
dataSet = 'bee_info_5_new';
file = fopen([dataSet,'/all/trajectory_info.csv']);
longDistanceThreshold = 1000;
highSpeedThreshold = 40;
slowSpeedThreshold = 30;
highDetectedTimeThreshold = 50;
G2Start = 1;
div = 1;


title = fgetl(file);

dateTimeList = {};

trajectoryCountG1 = {};
trajectoryCountG2 = {};
trajectoryCountG3 = {};
trajectoryCountG4 = {};

detectedTimeG1 = {};
detectedTimeG2 = {};
detectedTimeG3 = {};
detectedTimeG4 = {};
distanceG1 = {};
distanceG2 = {};
distanceG3 = {};
distanceG4 = {};
velocityG1 = {};
velocityG2 = {};
velocityG3 = {};
velocityG4 = {};

IDListG1 = {};
IDListG2 = {};
IDListG3 = {};
IDListG4 = {};


longDistanceTrajectoryCountG1 = {};
longDistanceTrajectoryCountG2 = {};
longDistanceTrajectoryCountG3 = {};
longDistanceTrajectoryCountG4 = {};
highSpeedTrajectoryCountG1 = {};
highSpeedTrajectoryCountG2 = {};
highSpeedTrajectoryCountG3 = {};
highSpeedTrajectoryCountG4 = {};
highDetectedTimeTrajectoryCountG1 = {};
highDetectedTimeTrajectoryCountG2 = {};
highDetectedTimeTrajectoryCountG3 = {};
highDetectedTimeTrajectoryCountG4 = {};
slowLongTrajectoryG1 = {};
slowLongTrajectoryG2 = {};
slowLongTrajectoryG3 = {};
slowLongTrajectoryG4 = {};

staticRatioG1 = {};
staticRatioG2 = {};
staticRatioG3 = {};
staticRatioG4 = {};
loiteringRatioG1 = {};
loiteringRatioG2 = {};
loiteringRatioG3 = {};
loiteringRatioG4 = {};
movingRatioG1 = {};
movingRatioG2 = {};
movingRatioG3 = {};
movingRatioG4 = {};
staticG1 = {};
staticG2 = {};
staticG3 = {};
staticG4 = {};
loiteringG1 = {};
loiteringG2 = {};
loiteringG3 = {};
loiteringG4 = {};
movingG1 = {};
movingG2 = {};
movingG3 = {};
movingG4 = {};

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
    
    
    dateTime = [month,'/',day];
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
        trajectoryCountG3{newID} = 0;
        trajectoryCountG4{newID} = 0;
        distanceG1{newID} = 0;
        distanceG2{newID} = 0;
        distanceG3{newID} = 0;
        distanceG4{newID} = 0;
        velocityG1{newID} = 0;
        velocityG2{newID} = 0;
        velocityG3{newID} = 0;
        velocityG4{newID} = 0;
        
        IDListG1{newID} = [];
        IDListG2{newID} = [];
        IDListG3{newID} = [];
        IDListG4{newID} = [];
        longDistanceTrajectoryCountG1{newID} = 0;
        longDistanceTrajectoryCountG2{newID} = 0;
        longDistanceTrajectoryCountG3{newID} = 0;
        longDistanceTrajectoryCountG4{newID} = 0;
        highSpeedTrajectoryCountG1{newID} = 0;
        highSpeedTrajectoryCountG2{newID} = 0;
        highSpeedTrajectoryCountG3{newID} = 0;
        highSpeedTrajectoryCountG4{newID} = 0;
        highDetectedTimeTrajectoryCountG1{newID} = 0;
        highDetectedTimeTrajectoryCountG2{newID} = 0;
        highDetectedTimeTrajectoryCountG3{newID} = 0;
        highDetectedTimeTrajectoryCountG4{newID} = 0;
        detectedTimeG1{newID} = 0;
        detectedTimeG2{newID} = 0;
        detectedTimeG3{newID} = 0;
        detectedTimeG4{newID} = 0;
        slowLongTrajectoryG1{newID} = 0;
        slowLongTrajectoryG2{newID} = 0;
        slowLongTrajectoryG3{newID} = 0;
        slowLongTrajectoryG4{newID} = 0;
        
        
        staticRatioG1{newID} = 0;
        staticRatioG2{newID} = 0;
        staticRatioG3{newID} = 0;
        staticRatioG4{newID} = 0;
        loiteringRatioG1{newID} = 0;
        loiteringRatioG2{newID} = 0;
        loiteringRatioG3{newID} = 0;
        loiteringRatioG4{newID} = 0;
        movingRatioG1{newID} = 0;
        movingRatioG2{newID} = 0;
        movingRatioG3{newID} = 0;
        movingRatioG4{newID} = 0;
        staticG1{newID} = [];
        staticG2{newID} = [];
        staticG3{newID} = [];
        staticG4{newID} = [];
        loiteringG1{newID} = [];
        loiteringG2{newID} = [];
        loiteringG3{newID} = [];
        loiteringG4{newID} = [];
        movingG1{newID} = [];
        movingG2{newID} = [];
        movingG3{newID} = [];
        movingG4{newID} = [];
        
        inList = 1;
        dateTimeID = newID;
        dateTime
    end
    
    
    
    if group == 1
        inSub = 0;
        for k = 1:size(subG1,1)
            if ID == subG1{k}
                inSub = 1;
                break;
            end
        end
        if inSub == 0
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
            
        elseif inSub == 1
            trajectoryCountG3{dateTimeID} = trajectoryCountG3{dateTimeID} + 1;
            detectedTimeG3{newID} = detectedTimeG3{newID} + detectedTime;
            
            if distance > longDistanceThreshold
                longDistanceTrajectoryCountG3{dateTimeID} = longDistanceTrajectoryCountG3{dateTimeID} + 1;
            end
            if velocity > highSpeedThreshold
                highSpeedTrajectoryCountG3{dateTimeID} = highSpeedTrajectoryCountG3{dateTimeID} + 1;
            end
            if detectedTime > highDetectedTimeThreshold
                highDetectedTimeTrajectoryCountG3{dateTimeID} = highDetectedTimeTrajectoryCountG3{dateTimeID} + 1;
            end
            
            if velocity < slowSpeedThreshold && distance > longDistanceThreshold
                slowLongTrajectoryG3{dateTimeID} = slowLongTrajectoryG3{dateTimeID} + 1;
            end
            distanceG3{newID} = distanceG3{newID}+distance;
            velocityG3{newID} = velocityG3{newID}+velocity;
            
            staticRatioG3{newID} = staticRatioG3{newID}+staticRatio;
            loiteringRatioG3{newID} = loiteringRatioG3{newID}+loiteringRatio;
            movingRatioG3{newID} = movingRatioG3{newID}+movingRatio;
            
            staticG3{newID} = [staticG3{newID},staticRatio];
            loiteringG3{newID} = [loiteringG3{newID},loiteringRatio];
            movingG3{newID} = [movingG3{newID},movingRatio];
            
            inIDList = 0;
            
            for m = 1:size(IDListG3{newID},1)
                if ID == IDListG3{newID}(m,:)
                    inIDList = 1;
                    break;
                end
            end
            
            if inIDList == 0 || size(IDListG3{newID},1) == 0
                IDListG3{newID} = [IDListG3{newID};ID];
            end
        end
        
    elseif group == 2
        
        inSub = 0;
        for k = 1:size(subG2,1)
            if ID == subG2{k}
                inSub = 1;
                break;
            end
        end
        if inSub == 0
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

        elseif inSub == 1
            detectedTimeG4{newID} = detectedTimeG4{newID} + detectedTime;
            trajectoryCountG4{dateTimeID} = trajectoryCountG4{dateTimeID} + 1;
            if distance > longDistanceThreshold
                longDistanceTrajectoryCountG4{dateTimeID} = longDistanceTrajectoryCountG4{dateTimeID} + 1;
            end
            if velocity > highSpeedThreshold
                highSpeedTrajectoryCountG4{dateTimeID} = highSpeedTrajectoryCountG4{dateTimeID} + 1;
            end
            if detectedTime > highDetectedTimeThreshold
                highDetectedTimeTrajectoryCountG4{dateTimeID} = highDetectedTimeTrajectoryCountG4{dateTimeID} + 1;
            end
            if velocity < slowSpeedThreshold && distance > longDistanceThreshold
                slowLongTrajectoryG4{dateTimeID} = slowLongTrajectoryG4{dateTimeID} + 1;
            end
            staticRatioG4{newID} = staticRatioG4{newID}+staticRatio;
            loiteringRatioG4{newID} = loiteringRatioG4{newID}+loiteringRatio;
            movingRatioG4{newID} = movingRatioG4{newID}+movingRatio;
            distanceG4{newID} = distanceG4{newID}+distance;
            velocityG4{newID} = velocityG4{newID}+velocity;
            
            staticG4{newID} = [staticG4{newID},staticRatio];
            loiteringG4{newID} = [loiteringG4{newID},loiteringRatio];
            movingG4{newID} = [movingG4{newID},movingRatio];
            
            inIDList = 0;
            for m = 1:size(IDListG4{newID},1)
                if ID == IDListG4{newID}(m,:)
                    inIDList = 1;
                    break;
                end
            end
            
            if inIDList == 0|| size(IDListG4{newID},1) == 0
                IDListG4{newID} = [IDListG4{newID};ID];
            end
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
IDListSizeG3 = [];
IDListSizeG4 = [];
for i = 1:size(IDListG1,2)
    IDListSizeG1 = [IDListSizeG1, size(IDListG1{i},1)];
    IDListSizeG2 = [IDListSizeG2, size(IDListG2{i},1)];
	IDListSizeG3 = [IDListSizeG3, size(IDListG3{i},1)];
    IDListSizeG4 = [IDListSizeG4, size(IDListG4{i},1)];
end
xCount = linspace(1,size(dateTimeList,2),size(dateTimeList,2));
xTick = linspace(1,size(dateTimeList,2),size(dateTimeList,2)/div);
xLabel = cell2mat(dateTimeList');
xLabel = xLabel(floor(xTick),:);
% plot long distance trajectory bar plot
longDistanceRatioA = cell2mat(longDistanceTrajectoryCountG1)./cell2mat(trajectoryCountG1);
% longDistanceRatioA = cell2mat(longDistanceTrajectoryCountG2)./cell2mat(trajectoryCountG2);
longDistanceRatioC = cell2mat(longDistanceTrajectoryCountG3)./cell2mat(trajectoryCountG3);
% longDistanceRatioC = cell2mat(longDistanceTrajectoryCountG4)./cell2mat(trajectoryCountG4);

% longDistanceRatioA = (longDistanceRatioA+longDistanceRatioB);
% longDistanceRatioC = (longDistanceRatioC+longDistanceRatioD);

aw = figure(1);
scatter(xCount(~isnan(longDistanceRatioA)),longDistanceRatioA(~isnan(longDistanceRatioA)),'o')
hold on
scatter(xCount(~isnan(longDistanceRatioC)),longDistanceRatioC(~isnan(longDistanceRatioC)),'x')
hold on
m = polyfit(xCount(~isnan(longDistanceRatioA)),longDistanceRatioA(~isnan(longDistanceRatioA)),1);
plot(xCount(~isnan(longDistanceRatioA)),xCount(~isnan(longDistanceRatioA)).*m(1)+m(2),'b','LineWidth', 1);
hold on
m = polyfit(xCount(~isnan(longDistanceRatioC)),longDistanceRatioC(~isnan(longDistanceRatioC)),1);
plot(xCount(~isnan(longDistanceRatioC)),xCount(~isnan(longDistanceRatioC)).*m(1)+m(2),'r','LineWidth', 1);
ylabel('Long Distance Trajectory Ratio');
legend('Normal Interactive', 'Highly Interactive')
set(gca,'XTick',xTick);
set(gca,'XTickLabel',xLabel);
set(gca,'XTickLabelRotation',30);
% subplot(2,1,2);
% ab = bar([1:size(longDistanceRatioB,2)],[longDistanceRatioB;longDistanceRatioD]');
% ab(1).FaceColor = 'b';
% ab(2).FaceColor = 'r';
% ylabel('Long Distance Trajectory Ratio');
% legend('Normal Interactive', 'Highly Interactive')
set(gca,'XTick',xTick);
set(gca,'XTickLabel',xLabel);
set(gca,'XTickLabelRotation',30);
set(aw, 'Position', [0 0 600 300]);
img = getframe(gcf);
imwrite(img.cdata, [dataSet,'/out/temp/long_distance_ratio.png']);
% saveas(gca,[dataSet,'/out/temp/long_distance_ratio.png']);


% plot high speed trajectory bar plot
highSpeedRatioA = cell2mat(highSpeedTrajectoryCountG1)./cell2mat(trajectoryCountG1);
% highSpeedRatioA = cell2mat(highSpeedTrajectoryCountG2)./cell2mat(trajectoryCountG2);
highSpeedRatioC = cell2mat(highSpeedTrajectoryCountG3)./cell2mat(trajectoryCountG3);
% highSpeedRatioC = cell2mat(highSpeedTrajectoryCountG4)./cell2mat(trajectoryCountG4);

% highSpeeeRatioA = highSpeedRatioA+highSpeedRatioC;
% highSpeedRatioB = highSpeedRatioB+highSpeedRatioD;

aw = figure(2);
% subplot(2,1,1);
scatter(xCount(~isnan(highSpeedRatioA)),highSpeedRatioA(~isnan(highSpeedRatioA)),'o')
hold on
scatter(xCount(~isnan(highSpeedRatioC)),highSpeedRatioC(~isnan(highSpeedRatioC)),'x')
hold on
m = polyfit(xCount(~isnan(highSpeedRatioA)),highSpeedRatioA(~isnan(highSpeedRatioA)),1);
plot(xCount(~isnan(highSpeedRatioA)),xCount(~isnan(highSpeedRatioA)).*m(1)+m(2),'b','LineWidth', 1);
hold on
m = polyfit(xCount(~isnan(highSpeedRatioC)),highSpeedRatioC(~isnan(highSpeedRatioC)),1);
plot(xCount(~isnan(highSpeedRatioC)),xCount(~isnan(highSpeedRatioC)).*m(1)+m(2),'r','LineWidth', 1);
ylabel('High Speed Trajectory Ratio');
legend('Normal Interactive', 'Highly Interactive')
set(gca,'XTick',xTick);
set(gca,'XTickLabel',xLabel);
set(gca,'XTickLabelRotation',30);
% subplot(2,1,2);
% ab = bar([1:size(highSpeedRatioB,2)],[highSpeedRatioB;highSpeedRatioD]');
% ab(1).FaceColor = 'b';
% ab(2).FaceColor = 'r';
% ylabel('High Speed Trajectory Ratio');
% legend('Normal Interactive', 'Highly Interactive')
set(gca,'XTick',xTick);
set(gca,'XTickLabel',xLabel);
set(gca,'XTickLabelRotation',30);
set(aw, 'Position', [0 0 600 300]);
img = getframe(gcf);
imwrite(img.cdata, [dataSet,'/out/temp/high_speed_ratio.png']);
% saveas(awigh_,[dataSet,'/out/temp/hspeed_ratio.png']);

% plot high detected time trajectory bar plot
highDetectedTimeRatioA = cell2mat(highDetectedTimeTrajectoryCountG1)./cell2mat(trajectoryCountG1);
% highDetectedTimeRatioA = cell2mat(highDetectedTimeTrajectoryCountG2)./cell2mat(trajectoryCountG2);
highDetectedTimeRatioC = cell2mat(highDetectedTimeTrajectoryCountG3)./cell2mat(trajectoryCountG3);
% highDetectedTimeRatioC = cell2mat(highDetectedTimeTrajectoryCountG4)./cell2mat(trajectoryCountG4);

% highDetectedTimeRatioA = highDetectedTimeRatioA+highDetectedTimeRatioC;
% highDetectedTimeRatioB = highDetectedTimeRatioB+highDetectedTimeRatioD;

aw = figure(3);
% subplot(2,1,1);
scatter(xCount(~isnan(highDetectedTimeRatioA)),highDetectedTimeRatioA(~isnan(highDetectedTimeRatioA)),'o')
hold on
scatter(xCount(~isnan(highDetectedTimeRatioC)),highDetectedTimeRatioC(~isnan(highDetectedTimeRatioC)),'x')
hold on
m = polyfit(xCount(~isnan(highDetectedTimeRatioA)),highDetectedTimeRatioA(~isnan(highDetectedTimeRatioA)),1);
plot(xCount(~isnan(highDetectedTimeRatioA)),xCount(~isnan(highDetectedTimeRatioA)).*m(1)+m(2),'b','LineWidth', 1);
hold on
m = polyfit(xCount(~isnan(highDetectedTimeRatioC)),highDetectedTimeRatioC(~isnan(highDetectedTimeRatioC)),1);
plot(xCount(~isnan(highDetectedTimeRatioC)),xCount(~isnan(highDetectedTimeRatioC)).*m(1)+m(2),'r','LineWidth', 1);
ylabel('High Detected Time Trajectory Ratio');
legend('Normal Interactive', 'Highly Interactive')
set(gca,'XTick',xTick);
set(gca,'XTickLabel',xLabel);
set(gca,'XTickLabelRotation',30);
% subplot(2,1,2);
% ab = bar([1:size(highDetectedTimeRatioB,2)],[highDetectedTimeRatioB;highDetectedTimeRatioD]');
% ab(1).FaceColor = 'b';
% ab(2).FaceColor = 'r';
% ylabel('High Detected Time Trajectory Ratio');
% legend('Normal Interactive', 'Highly Interactive')
set(gca,'XTick',xTick);
set(gca,'XTickLabel',xLabel);
set(gca,'XTickLabelRotation',30);
set(aw, 'Position', [0 0 600 300]);
img = getframe(gcf);
imwrite(img.cdata,[dataSet,'/out/temp/high_detected_ratio.png']);
% saveas(gca,[dataSet,'/out/temp/high_detected_ratio.png']);


staticVarG1 = [];
staticVarG2 = [];
staticVarG3 = [];
staticVarG4 = [];
for i = 1:size(staticG1,2)
    staticVarG1 = [staticVarG1,var(staticG1{i})];
    staticVarG2 = [staticVarG2,var(staticG2{i})];
    staticVarG1 = [staticVarG3,var(staticG3{i})];
    staticVarG2 = [staticVarG4,var(staticG4{i})];
end

% static ratio
staticRatioA = cell2mat(staticRatioG1)./cell2mat(trajectoryCountG1);
% staticRatioA = cell2mat(staticRatioG2)./cell2mat(trajectoryCountG2);
staticRatioC = cell2mat(staticRatioG3)./cell2mat(trajectoryCountG3);
% staticRatioC = cell2mat(staticRatioG4)./cell2mat(trajectoryCountG4);

% staticRatioA = staticRatioA+staticRatioC;
% staticRatioB =staticRatioB+staticRatioD;

aw = figure(4);
% subplot(2,1,1)
ab = bar([1:size(staticRatioA,2)],[staticRatioA;staticRatioC]');
ab(1).FaceColor = 'b';
ab(2).FaceColor = 'r';
ylabel('Static Pattern Ratio');
legend('Normal Interactive', 'Highly Interactive')
set(gca,'XTick',xTick);
set(gca,'XTickLabel',xLabel);
set(gca,'XTickLabelRotation',30);
% subplot(2,1,2)
% ab = bar([1:size(staticRatioB,2)],[staticRatioB;staticRatioD]');
% ab(1).FaceColor = 'b';
% ab(2).FaceColor = 'r';
% ylabel('Static Pattern Ratio');
% legend('Normal Interactive', 'Highly Interactive')
set(gca,'XTick',xTick);
set(gca,'XTickLabel',xLabel);
set(gca,'XTickLabelRotation',30);
set(aw, 'Position', [0 0 600 300]);
img = getframe(gcf);
imwrite(img.cdata,[dataSet,'/out/temp/static_pattern_ratio.png']);
% saveas(aw,[dataSet,'/out/temp/static_pattern_ratio.png']);


% loitering ratio
loiteringRatioA = cell2mat(loiteringRatioG1)./cell2mat(trajectoryCountG1);
% loiteringRatioA = cell2mat(loiteringRatioG2)./cell2mat(trajectoryCountG2);
loiteringRatioC = cell2mat(loiteringRatioG3)./cell2mat(trajectoryCountG3);
% loiteringRatioC = cell2mat(loiteringRatioG4)./cell2mat(trajectoryCountG4);

% loiteringRatioA = loiteringRatioA+loiteringRatioC;
% loiteringRatioB = loiteringRatioB+loiteringRatioD;

aw = figure(5);
% subplot(2,1,1)
ab = bar([1:size(loiteringRatioA,2)],[loiteringRatioA;loiteringRatioC]');
ab(1).FaceColor = 'b';
ab(2).FaceColor = 'r';
ylabel('Loitering Pattern Ratio');
legend('Normal Interactive', 'Highly Interactive')
set(gca,'YLim',[0.1 0.4],'YTick',0.1:0.1:0.4);
set(gca,'XTick',xTick);
set(gca,'XTickLabel',xLabel);
set(gca,'XTickLabelRotation',30);
set(aw, 'Position', [0 0 600 300]);
set(gca,'XTickLabelRotation',30);
% subplot(2,1,2)
% ab = bar([1:size(loiteringRatioB,2)],[loiteringRatioB;loiteringRatioD]');
% ab(1).FaceColor = 'b';
% ab(2).FaceColor = 'r';
% ylabel('Loitering Pattern Ratio');
% legend('Normal Interactive', 'Highly Interactive')
set(gca,'YLim',[0.1 0.4],'YTick',0.1:0.1:0.4);
set(gca,'XTick',xTick);
set(gca,'XTickLabel',xLabel);
set(gca,'XTickLabelRotation',30);
set(aw, 'Position', [0 0 600 300]);
set(gca,'XTickLabelRotation',30);
set(aw, 'Position', [0 0 600 300]);
img = getframe(gcf);
imwrite(img.cdata,[dataSet,'/out/temp/loitering_pattern_ratio.png']);
% saveas(aw,[dataSet,'/out/temp/loitering_pattern_ratio.png']);

% moving ratio
movingRatioA = cell2mat(movingRatioG1)./cell2mat(trajectoryCountG1);
% movingRatioA = cell2mat(movingRatioG2)./cell2mat(trajectoryCountG2);
movingRatioC = cell2mat(movingRatioG3)./cell2mat(trajectoryCountG3);
% movingRatioC = cell2mat(movingRatioG4)./cell2mat(trajectoryCountG4);

% movingRatioA = movingRatioA+movingRatioC;
% movingRatioB = movingRatioB+movingRatioD;

aw = figure(6);
% subplot(2,1,1);
ab = bar([1:size(movingRatioA,2)],[movingRatioA;movingRatioC]');
ab(1).FaceColor = 'b';
ab(2).FaceColor = 'r';
ylabel('Moving Pattern Ratio');
legend('Normal Interactive', 'Highly Interactive')
set(gca,'YLim',[0.3 0.8],'YTick',0.3:0.1:0.8);
set(gca,'XTick',xTick);
set(gca,'XTickLabel',xLabel);
set(gca,'XTickLabelRotation',30);
% subplot(2,1,2);
% ab = bar([1:size(movingRatioB,2)],[movingRatioB;movingRatioD]');
% ab(1).FaceColor = 'b';
% ab(2).FaceColor = 'r';
% ylabel('Moving Pattern Ratio');
% legend('Normal Interactive', 'Highly Interactive')
set(gca,'YLim',[0.3 0.8],'YTick',0.3:0.1:0.8);
set(gca,'XTick',xTick);
set(gca,'XTickLabel',xLabel);
set(gca,'XTickLabelRotation',30);

set(aw, 'Position', [0 0 600 300]);
img = getframe(gcf);
imwrite(img.cdata,[dataSet,'/out/temp/moving_pattern_ratio.png']);
% saveas(aw,[dataSet,'/out/temp/moving_pattern_ratio.png']);

% slow long trajectory
slowLongRatioA = cell2mat(slowLongTrajectoryG1)./cell2mat(trajectoryCountG1);
% slowLongRatioA = cell2mat(slowLongTrajectoryG2)./cell2mat(trajectoryCountG2);
slowLongRatioC = cell2mat(slowLongTrajectoryG3)./cell2mat(trajectoryCountG3);
% slowLongRatioC = cell2mat(slowLongTrajectoryG4)./cell2mat(trajectoryCountG4);

% slowLongRatioA = slowLongRatioA+slowLongRatioC;
% slowLongRatioB = slowLongRatioA+slowLongRatioD;

aw = figure(7);
% subplot(2,1,1);
scatter(xCount(~isnan(slowLongRatioA)),slowLongRatioA(~isnan(slowLongRatioA)),'o')
hold on
scatter(xCount(~isnan(slowLongRatioC)),slowLongRatioC(~isnan(slowLongRatioC)),'x')
hold on
m = polyfit(xCount(~isnan(slowLongRatioA)),slowLongRatioA(~isnan(slowLongRatioA)),1);
plot(xCount(~isnan(slowLongRatioA)),xCount(~isnan(slowLongRatioA)).*m(1)+m(2),'b','LineWidth', 1);
hold on
m = polyfit(xCount(~isnan(slowLongRatioC)),slowLongRatioC(~isnan(slowLongRatioC)),1);
plot(xCount(~isnan(slowLongRatioC)),xCount(~isnan(slowLongRatioC)).*m(1)+m(2),'r','LineWidth', 1);
ylabel('Slow and Long Trajectory Ratio');
legend('Normal Interactive', 'Highly Interactive')
set(gca,'XTick',xTick);
set(gca,'XTickLabel',xLabel);
set(gca,'XTickLabelRotation',30);
% subplot(2,1,2);
% ab = bar([1:size(slowLongRatioB,2)],[slowLongRatioB;slowLongRatioD]');
% ab(1).FaceColor = 'b';
% ab(2).FaceColor = 'r';
% ylabel('Slow and Long Trajectory Ratio');
% legend('Normal Interactive', 'Highly Interactive')
set(gca,'XTick',xTick);
set(gca,'XTickLabel',xLabel);
set(gca,'XTickLabelRotation',30);
set(aw, 'Position', [0 0 600 300]);
img = getframe(gcf);
imwrite(img.cdata,[dataSet,'/out/temp/slow_long_ratio.png']);
% saveas(aw,[dataSet,'/out/temp/slow_long_ratio.png']);

% plot high detected time trajectory bar plot
detectedTimeA = cell2mat(detectedTimeG1)./IDListSizeG1;
% detectedTimeA = cell2mat(detectedTimeG2)./IDListSizeG2;
detectedTimeC = cell2mat(detectedTimeG3)./IDListSizeG3;
% detectedTimeC = cell2mat(detectedTimeG4)./IDListSizeG4;
% detectedTimeA = detectedTimeA+detectedTimeC;
% detectedTimeB = detectedTimeB+detectedTimeD;
aw = figure(8);
% subplot(2,1,1);
scatter(xCount(~isnan(detectedTimeA)),detectedTimeA(~isnan(detectedTimeA)),'o')
hold on
scatter(xCount(~isnan(detectedTimeC)),detectedTimeC(~isnan(detectedTimeC)),'x')
hold on
m = polyfit(xCount(~isnan(detectedTimeA)),detectedTimeA(~isnan(detectedTimeA)),1);
plot(xCount(~isnan(detectedTimeA)),xCount(~isnan(detectedTimeA)).*m(1)+m(2),'b','LineWidth', 1);
hold on
m = polyfit(xCount(~isnan(detectedTimeC)),detectedTimeC(~isnan(detectedTimeC)),1);
plot(xCount(~isnan(detectedTimeC)),xCount(~isnan(detectedTimeC)).*m(1)+m(2),'r','LineWidth', 1);
ylabel('Detected Time (sec)');
legend('Normal Interactive', 'Highly Interactive')
set(gca,'XTick',xTick);
set(gca,'XTickLabel',xLabel);
set(gca,'XTickLabelRotation',30);
% subplot(2,1,2);
% ab = bar([1:size(detectedTimeB,2)],[detectedTimeB;detectedTimeD]');
% ab(1).FaceColor = 'b';
% ab(2).FaceColor = 'r';
% ylabel('Detected Time (sec)');
% legend('Normal Interactive', 'Highly Interactive')
set(gca,'XTick',xTick);
set(gca,'XTickLabel',xLabel);
set(gca,'XTickLabelRotation',30);

set(aw, 'Position', [0 0 600 300]);
img = getframe(gcf);
imwrite(img.cdata,[dataSet,'/out/temp/detected_time.png']);
% saveas(aw,[dataSet,'/out/temp/detected_time.png']);

ratio = 0.001125;

% plot distance bar plot
distanceA = cell2mat(distanceG1)./IDListSizeG1.*ratio;
% distanceA = cell2mat(distanceG2)./IDListSizeG2.*ratio;
distanceC = cell2mat(distanceG3)./IDListSizeG3.*ratio;
% distanceC = cell2mat(distanceG4)./IDListSizeG4.*ratio;

% distanceA = distanceA+distanceC;
% distanceB = distanceB+distanceD;

aw = figure(9);
% subplot(2,1,1);
scatter(xCount(~isnan(distanceA)),distanceA(~isnan(distanceA)),'o')
hold on
scatter(xCount(~isnan(distanceC)),distanceC(~isnan(distanceC)),'x')
hold on
m = polyfit(xCount(~isnan(distanceA)),distanceA(~isnan(distanceA)),1);
plot(xCount(~isnan(distanceA)),xCount(~isnan(distanceA)).*m(1)+m(2),'b','LineWidth', 1);
hold on
m = polyfit(xCount(~isnan(distanceC)),distanceC(~isnan(distanceC)),1);
plot(xCount(~isnan(distanceC)),xCount(~isnan(distanceC)).*m(1)+m(2),'r','LineWidth', 1);
ylabel('Distance (cm)');
legend('Normal Interactive', 'Highly Interactive')
set(gca,'XTick',xTick);
set(gca,'XTickLabel',xLabel);
set(gca,'XTickLabelRotation',30);
% subplot(2,1,2);
% ab = bar([1:size(distanceB,2)],[distanceB;distanceD]');
% ab(1).FaceColor = 'b';
% ab(2).FaceColor = 'r';
% ylabel('Distance (pixels)');
% legend('Normal Interactive', 'Highly Interactive')
set(gca,'XTick',xTick);
set(gca,'XTickLabel',xLabel);
set(gca,'XTickLabelRotation',30);

set(aw, 'Position', [0 0 600 300]);
img = getframe(gcf);
imwrite(img.cdata,[dataSet,'/out/temp/distance.png']);
% saveas(aw,[dataSet,'/out/temp/distance.png']);

% plot velocity bar plot
velocityA = cell2mat(velocityG1)./IDListSizeG1.*ratio;
% velocityA = cell2mat(velocityG2)./IDListSizeG2.*ratio;
velocityC = cell2mat(velocityG3)./IDListSizeG3.*ratio;
% velocityC = cell2mat(velocityG4)./IDListSizeG4.*ratio;

% velocityA = velocityA+velocityC;
% velocityB = velocityA+velocityD;

aw = figure(10);
% subplot(2,1,1);
scatter(xCount(~isnan(velocityA)),velocityA(~isnan(velocityA)),'o')
hold on
scatter(xCount(~isnan(velocityC)),velocityC(~isnan(velocityC)),'x')
hold on
m = polyfit(xCount(~isnan(velocityA)),velocityA(~isnan(velocityA)),1);
plot(xCount(~isnan(velocityA)),xCount(~isnan(velocityA)).*m(1)+m(2),'b','LineWidth', 1);
hold on
m = polyfit(xCount(~isnan(velocityC)),velocityC(~isnan(velocityC)),1);
plot(xCount(~isnan(velocityC)),xCount(~isnan(velocityC)).*m(1)+m(2),'r','LineWidth', 1);
ylabel('Velocity (cm/sec)');
legend('Normal Interactive', 'Highly Interactive')
set(gca,'XTick',xTick);
set(gca,'XTickLabel',xLabel);
set(gca,'XTickLabelRotation',30);
% subplot(2,1,2);
% ab = bar([1:size(velocityB,2)],[velocityB;velocityD]');
% ab(1).FaceColor = 'b';
% ab(2).FaceColor = 'r';
% ylabel('Velocity (pixels)');
% legend('Normal Interactive', 'Highly Interactive')
set(gca,'XTick',xTick);
set(gca,'XTickLabel',xLabel);
set(gca,'XTickLabelRotation',30);
set(aw, 'Position', [0 0 600 300]);
img = getframe(gcf);
imwrite(img.cdata,[dataSet,'/out/temp/velocity.png']);
% saveas(aw,[dataSet,'/out/temp/velocity.png']);