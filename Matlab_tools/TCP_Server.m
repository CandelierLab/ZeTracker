clc

% === Parameters ==========================================================

port = 3231;

% =========================================================================

fprintf('\n=== TCP/IP server ===\n\n');

while true
    
    conn = tcpip('0.0.0.0', port, 'NetworkRole', 'server');
    
    fprintf('Waiting for a connection ...');
    fopen(conn);
    fprintf(' connection succesfully established.\n');
    
    while true
        
        if conn.BytesAvailable
            
            data = char(fread(conn, conn.BytesAvailable))';
            fprintf('> %s\n', data);
                        
            if strcmp(data, 'quit')
                break;
            end
            
        end
        
    end
    
    fclose(conn);
    fprintf('Connection closed.\n');
    
end