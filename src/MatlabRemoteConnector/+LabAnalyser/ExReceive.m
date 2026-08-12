function data = ExReceive(id, port)
if nargin < 2
    port = 4080;
end

portPointer = libpointer('cstring', num2str(port));
idPointer = libpointer('cstring', char(id));
commandPointer = libpointer('cstring', 'get');
data = [];

elementCount = calllib('TCPClient', 'ReceiveDoubleData', ...
    idPointer, commandPointer, portPointer);
if elementCount <= 0
    return;
end

dataPointer = libpointer('doublePtr', ones(elementCount, 1));
text = calllib('TCPClient', 'ReadReceivedDoubleData', dataPointer, portPointer);
if ~isempty(text)
    data = text;
else
    data = dataPointer.Value;
end
end
