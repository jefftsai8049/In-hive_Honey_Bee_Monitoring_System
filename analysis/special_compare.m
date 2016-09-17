clc;
close all;
clear all;

% dataSet = 'bee_info_4';
dataSet = 'bee_info_5_new';
days = {'06-08';'06-09';'06-10';'06-11';'06-12';'06-13';'06-14';'06-15';'06-16';'06-17';'06-18';'06-19';'06-20';'06-21';'06-22';'06-23';'06-24';'06-25';'06-26';'06-27'};
for i = 1:size(days,1)
    fileName = [dataSet,'/days/',days{i},'/individual_behavior.csv'];
    
    [x{i}.ID,x{i}.Group,x{i}.MotionRatioStatic,x{i}.MotionRatioLoitering,x{i}.MotionRatioMovingForward,x{i}.MotionRatioMovingCW,x{i}.MotionRatioMovingCCW,x{i}.Waggle,x{i}.Velocity,x{i}.Distance,x{i}.AvgLoiteringTime,x{i}.AvgStaticTime,x{i}.AvgMovingTime,x{i}.DetectedTime,x{i}.TrajectoryCount] = importfile_behavior(fileName,2,inf);
end
subG1 = {'AR';'BB';'BO';'BP';'BR';'BY';'BZ';'CH';'CP';'FE';'FF';'FL';'GP'};
% subG1 = {'AF';'AO';'AR';'BA';'BB';'BG';'BK';'BL';'BO';'BP';'BR';'BS';'BU';'BY';'BZ';'CA';'CG';'CH';'CK';'CP';'EA';'FC';'FE';'FF';'FL';'FT';'FZ';'GL';'GP';};

for i = 1:size(subG1,1)
   count = [];
   val = [];
   group = 0;
   for j = 1:size(x,2)
       for k = 1:size(x{j}.ID,1)
           if x{j}.ID{k} == subG1{i}
               count = [count, j];
               val = [val , x{j}.MotionRatioLoitering(k)];
%            else
%                count = [count, j];
%                val = [val , nan];
           end
       end
   end
    plot(count,val)
    hold on
     
end