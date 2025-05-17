
% SD card path
sdCardPath = 'F:\';  % Adjust as needed

% File list and joint names
fileList = {'SHOULDER.txt', 'WRIST.txt', 'ELBOW.txt'};
jointNames = {'Shoulder', 'Wrist', 'Elbow'};

% Define test order and corresponding normal ROM values
normROMs = {
    [180, 50, 30, 180],   % Shoulder: Flexion, Extension, Adduction, Abduction
    [85, 85],             % Wrist: Flexion, Extension
    [55, 90, 85, 90]      % Elbow: Flexion, Extension, Pronation, Supination
};

% Define MAS scoring logic
computeMAS = @(p) ...
    0   * (p > 90)   + ...
    1   * (p <= 90 & p > 75) + ...
    1.5 * (p <= 75 & p > 50) + ...
    2   * (p <= 50 & p > 25) + ...
    3   * (p <= 25 & p > 10) + ...
    4   * (p <= 10);

% Loop through each file/joint
for i = 1:numel(fileList)
    filePath = fullfile(sdCardPath, fileList{i});
    
    if ~isfile(filePath)
        warning('File not found: %s. Skipping...', filePath);
        continue;
    end

    fid = fopen(filePath, 'r');
    
    testLabels = {};
    romValues = [];
    masScores = [];
    normalROMs = normROMs{i};  % Get normal ROMs for this joint
    testIndex = 0;             % Track position in normal ROM array

    while ~feof(fid)
        line = strtrim(fgetl(fid));
        
        if startsWith(line, 'TEST')
            parts = strsplit(line, ',');
            testName = strtrim(parts{2});
            testLabels{end+1} = testName;
            testIndex = testIndex + 1;
        elseif startsWith(line, 'ROM')
            parts = strsplit(line, ':');
            rom = str2double(strtrim(parts{2}));
            romValues(end+1) = rom;

            % Use the correct normal ROM
            if testIndex <= numel(normalROMs)
                normROM = normalROMs(testIndex);
            else
                warning('More ROM values than expected for %s', jointNames{i});
                normROM = 180; % fallback
            end

            % Compute % of normal and MAS score
            percentROM = (rom / normROM) * 100;
            mas = computeMAS(percentROM);
            masScores(end+1) = mas;
        end
    end
    fclose(fid);

    % Plot ROM Bar Graph
    figure;
    bar(romValues);
    set(gca, 'XTickLabel', testLabels, 'XTick', 1:numel(romValues));
    xlabel('Test Type');
    ylabel('ROM (degrees)');
    title(sprintf('ROM - %s', jointNames{i}));
    grid on;

    saveas(gcf, sprintf('%s.png', jointNames{i}));

    % Display MAS scores in command window
    fprintf('\n%s MAS Scores:\n', jointNames{i});
    for j = 1:numel(testLabels)
        fprintf('%s: ROM = %.2f°, Normal = %.2f°, %%ROM = %.1f%%, MAS = %s\n', ...
            testLabels{j}, romValues(j), normalROMs(j), ...
            (romValues(j) / normalROMs(j)) * 100, num2str(masScores(j)));
    end
end
