function result = ExSendDouble(id, value, port)
if nargin < 3
    port = 4080;
end

portPointer = libpointer('cstring', num2str(port));
idPointer = libpointer('cstring', char(id));
% The wire contract accepts exactly one native double per set request.
dataPointer = libpointer('doublePtr', value(1));
result = calllib('TCPClient', 'SendDoubleData', idPointer, dataPointer, portPointer);
end
