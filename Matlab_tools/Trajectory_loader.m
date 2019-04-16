clc

% === Parameters ==========================================================

runPath = '/home/ljp/Science/ZeBeauty/Data/2019-04-16/Run test/';

% =========================================================================

% --- Open file
trajPath = [runPath 'Trajectory.dat'];
fid = fopen(trajPath);

% --- Get version
version = fread(fid, 1, 'double', 0, 'b');

switch version
    
    case 1.0
        
        ncol = 4;
        
        t = uint64.empty;
        A = double.empty(0, ncol-1);
        
        fid = fopen(trajPath);
        while true
            try
                t(end+1,1) = fread(fid, 1, 'uint64=>uint64', 0, 'b');
                A(end+1,:) = fread(fid, ncol-1, 'double', 0, 'b');
            catch
                if numel(t)>size(A,1), t(end) = []; end
                break;
            end
        end
        
end

fclose(fid);

size(t)
size(A)