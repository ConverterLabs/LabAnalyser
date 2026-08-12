function Connect(port)
if nargin < 1
    port = 4080;
end

packageDirectory = fileparts(mfilename('fullpath'));
dllPath = fullfile(packageDirectory, 'TCPClient.dll');
headerPath = fullfile(packageDirectory, 'TCPClient.h');

if ~libisloaded('TCPClient')
    loadlibrary(dllPath, headerPath, 'alias', 'TCPClient');
end

portPointer = libpointer('cstring', num2str(port));
calllib('TCPClient', 'Disconnect', portPointer);
if calllib('TCPClient', 'Connect', portPointer)
    error('LabAnalyser:ConnectionFailed', ...
        'Couldn''t connect to LabAnalyser. Check whether it is running and which remote port it uses.');
end
end
