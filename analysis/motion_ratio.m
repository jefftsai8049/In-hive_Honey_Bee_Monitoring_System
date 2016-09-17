clc;
close all;
clear all;

% dataSet = 'bee_info_4';
dataSet = 'bee_info_5_new';
days = {'06-08';'06-09';'06-10';'06-11';'06-12';'06-13';'06-14';'06-15';'06-16';'06-17';'06-18';'06-19';'06-20';'06-21';'06-22';'06-23';'06-24';'06-25';'06-26';'06-27'};
% days = {'12-29';'12-30';'12-31';'01-01';'01-02';'01-03';'01-04';'01-05'};
% days = {'05-03';'05-04';'05-05';'05-06';'05-07';'05-08';'05-09';'05-10';'05-11';'05-12';'05-13';'05-14';'05-15';'05-16';'05-17';'05-18';};
% days = {'05-03';'05-04';'05-05';'05-06';'05-07';'05-08';'05-09';'05-10';'05-11';'05-12';'05-13';'05-14';'05-15';'05-16';'05-17';'05-18';};
for i = 1:size(days,1)
    fileName = [dataSet,'/days/',days{i},'/individual_info.csv'];
    
    [labelY{i},x{i}(:,1),x{i}(:,2),x{i}(:,3),x{i}(:,4),x{i}(:,5),x{i}(:,6)] = importfile6D(fileName,1,inf);
    breakPoint{i} = size(labelY{i},1);
    for j = 1 : size(labelY{i})
        if labelY{i}{j,1}(1) == 'H'||labelY{i}{j,1}(1) =='L'||labelY{i}{j,1}(1) =='K'||labelY{i}{j,1}(1) =='O'||labelY{i}{j,1}(1) =='P'||labelY{i}{j,1}(1) =='R'
            %         if labelY{i}{j,1}(1) == 'E'||labelY{i}{j,1}(1) =='F'|| labelY{i}{j,1}(1) =='G'
            breakPoint{i} = j;
            break
        end
    end
end
%%
clear i j

targetID = 'AA';
for i = 1:size(labelY,2)
    for j = 1:size(labelY{i},1)
        if targetID == labelY{i}{j}
            targetIDIndex{i} = j;
            break
        end
        targetIDIndex{i} = 0;
    end
end
%%

data = zeros(size(labelY,2),6);
for i = 1:size(labelY,2)
    if targetIDIndex{i} > 0
        data(i,:) = x{i}(targetIDIndex{i},:);

    subplot(4,ceil(size(labelY,2)/4),i);
    pie(data(i,:));
    end
end

%%
sumG1 = [];
varG1 = [];
sumG2 = [];
varG2 = [];
for i = 1:size(days,1)
    sumG1 = [sumG1;sum(x{i}(1:breakPoint{i},:),1)./size(x{i}(1:breakPoint{i},:),1)];
    varG1 = [varG1;var(x{i}(1:breakPoint{i},:))];
    sumG2 = [sumG2;sum(x{i}(breakPoint{i}+1:end,:),1)./size(x{i}(breakPoint{i}+1:end,:),1)];
    varG2 = [varG2;var(x{i}(breakPoint{i}+1:end,:))];
end



figure(1)
subplot(2,1,1);
ap =gca;
errorbar(sumG1(:,1),varG1(:,1),'-or')
legend('Field bee');
set(ap,'XTick',linspace(1,size(days,1),size(days,1)));
set(ap,'XTickLabel',days');
ap.XTickLabelRotation=30;
subplot(2,1,2);
ap =gca;
errorbar(sumG2(:,1),varG2(:,1),'-xb')
legend('In-hive bee');
% legend('Age D+7','Age D+0');
set(ap,'XTick',linspace(1,size(days,1),size(days,1)));
set(ap,'XTickLabel',days');
ap.XTickLabelRotation=30;

figure(2)
subplot(2,1,1);
bp =gca;
errorbar(sumG1(:,2),varG1(:,2),'-ro')
legend('Field bee');
set(bp,'XTick',linspace(1,size(days,1),size(days,1)));
set(bp,'XTickLabel',days');
bp.XTickLabelRotation=30;
subplot(2,1,2);
bp = gca;
errorbar(sumG2(:,2),varG2(:,2),'-bx')
legend('In-hive bee');
% legend('Age D+7','Age D+0');
set(bp,'XTick',linspace(1,size(days,1),size(days,1)));
set(bp,'XTickLabel',days');
bp.XTickLabelRotation=30;

figure(4)
subplot(2,1,1);
cp = gca;
errorbar(sum(sumG1(:,3:6)'),sum(varG1(:,3:6)'),'-ro')
legend('Field bee');
set(cp,'XTick',linspace(1,size(days,1),size(days,1)));
set(cp,'XTickLabel',days');
cp.XTickLabelRotation=30;
subplot(2,1,2);
cp = gca;
errorbar(sum(sumG2(:,3:6)'),sum(varG2(:,3:6)'),'-xb')
legend('In-hive bee');
% legend('Age D+7','Age D+0');
set(cp,'XTick',linspace(1,size(days,1),size(days,1)));
set(cp,'XTickLabel',days');
cp.XTickLabelRotation=30;