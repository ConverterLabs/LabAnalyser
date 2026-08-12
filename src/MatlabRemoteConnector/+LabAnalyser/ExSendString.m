function result = ExSendString(id, value, port)
if nargin < 3
    port = 4080;
end

portPointer = libpointer('cstring', num2str(port));
idPointer = libpointer('cstring', char(id));
dataPointer = libpointer('cstring', char(value));
result = calllib('TCPClient', 'SendStringData', idPointer, dataPointer, portPointer);
end
