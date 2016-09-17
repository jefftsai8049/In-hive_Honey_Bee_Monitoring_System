
file = fopen(fileName);

groupAID = {'A';'B';'C';'E';'F';'G'};
% groupBID = {'A';'B';'C'};
groupBID = {'H';'L';'K';'O';'P';'R'};
% groupAID = {'E','F','G'};

str = fgetl(file);
dayhourList = {};

distanceA = {};
distanceB = {};

velocityA = {};
velocityB = {};

IDsAG = {};
IDsBG = {};

trajectoryA = {};
trajectoryB = {};

datehourLabel = {};

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
    
    dayhour = [day,'-',hour];
    
    for i = 1:size(dayhourList,2)
        if dayhourList{i} == dayhour
            inList = 1;
            dayhourID = i;
            break;
        end
    end
    
    if inList == 0
        dayhourList{size(dayhourList,2)+1} = dayhour
        dayhourID = size(dayhourList,2);
        
        datehourLabel{dayhourID} = [month,'/',day,'-',hour];
        
        distanceA{dayhourID} = 0;
        velocityA{dayhourID} = 0;
        IDsAG{dayhourID} = 0;
        trajectoryA{dayhourID} = 0;
        
        distanceB{dayhourID} = 0;
        velocityB{dayhourID} = 0;
        IDsBG{dayhourID} = 0;
        trajectoryB{dayhourID} = 0;
        inList = 1;
        
    end
    
    if sum(strcmp(groupAID,ID(1))) > 0
        distanceA{dayhourID} = distanceA{dayhourID}+distance;
        velocityA{dayhourID} = velocityA{dayhourID}+velocity;
        trajectoryA{dayhourID} = trajectoryA{dayhourID}+1;
    elseif sum(strcmp(groupBID,ID(1))) > 0
        distanceB{dayhourID} = distanceB{dayhourID}+distance;
        velocityB{dayhourID} = velocityB{dayhourID}+velocity;
        trajectoryB{dayhourID} = trajectoryB{dayhourID}+1;
        
    end
end

%%
xDis= cell2mat(distanceA);
yDis = cell2mat(distanceB);
xTra = cell2mat(distanceA)./cell2mat(trajectoryA);
yTra = cell2mat(distanceB)./cell2mat(trajectoryB);
xV = cell2mat(velocityA)./cell2mat(trajectoryA);
yV = cell2mat(velocityB)./cell2mat(trajectoryB);


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
set(aw, 'Position', [0 0 600 300]);
ab = bar([1:size(distanceA,2)],[cell2mat(trajectoryA);cell2mat(trajectoryB)]');
ab(1).FaceColor = 'b';
ab(1).EdgeColor = 'b';
ab(2).FaceColor = 'g';
ab(2).EdgeColor = 'g';
ylabel('Trajectory counts');
% legend('Age D+7','Age D+0');
legend(controlLegendName,experimentLegendName);
% set(ap,'XTick',linspace(1,size(datehourLabel,2),size(datehourLabel,2)));
set(ap,'XTick',1:12:size(datehourLabel,2));
set(ap,'XTickLabel',datehourLabel(1:12:size(datehourLabel,2)));
set(ap,'XTickLabelRotation',30);

bw = figure(2);
% bw = subplot(3,1,2);
bp = gca;
set(bw, 'Position', [0 0 600 300]);
bb = bar([1:size(velocityA,2)],[xTra;yTra]');
bb(1).FaceColor = 'b';
bb(1).EdgeColor = 'b';
bb(2).FaceColor = 'g';
bb(2).EdgeColor = 'g';
% bar([1:size(velocityA,2)],xTra,'r');
% hold on
% bar([1:size(velocityA,2)],yTra,'b');

ylabel('Moving distance (cm)');
% legend('Age D+7','Age D+0');
legend(controlLegendName,experimentLegendName);
set(bp,'XTick',1:12:size(datehourLabel,2));
set(bp,'XTickLabel',datehourLabel(1:12:size(datehourLabel,2)));
set(bp,'XTickLabelRotation',30);

cw = figure(3);
% cw = subplot(3,1,3);
cp = gca;
set(cw, 'Position', [0 0 600 300]);
cb = bar([1:size(velocityA,2)],[xV;yV]');
cb(1).FaceColor = 'b';
cb(1).EdgeColor = 'b';
cb(2).FaceColor = 'g';
cb(2).EdgeColor = 'g';
ylabel('Moving speed (cm/sec)');
% legend('Age D+7','Age D+0');
legend(controlLegendName,experimentLegendName);
set(cp,'XTick',1:12:size(datehourLabel,2));
set(cp,'XTickLabel',datehourLabel(1:12:size(datehourLabel,2)));
set(cp,'XTickLabelRotation',30);