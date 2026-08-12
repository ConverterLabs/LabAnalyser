function Disconnect(port)
if nargin < 1
    port = 4080;
end

if ~libisloaded('TCPClient')
    return;
end

portPointer = libpointer('cstring', num2str(port));
calllib('TCPClient', 'Disconnect', portPointer);
unloadlibrary('TCPClient');
end
