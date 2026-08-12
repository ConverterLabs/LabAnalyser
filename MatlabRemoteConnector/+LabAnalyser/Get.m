function [y, x] = Get(id, port)
if nargin < 2
    port = 4080;
end

idText = char(id);
if ~isempty(strfind(idText, '*')) %#ok<STREMP>
    [y, x] = getWildcard(idText, port);
else
    [y, x] = getSingle(idText, port);
end
end

function [results, x] = getWildcard(pattern, port)
results = containers.Map('KeyType', 'char', 'ValueType', 'any');
x = [];

encodedIds = LabAnalyser.ExReceive(pattern, port);
assertConnected(port, pattern);
if isempty(encodedIds)
    return;
end

ids = strsplit(char(encodedIds), '|');
for index = 1:numel(ids)
    currentId = ids{index};
    [value, time] = getSingle(currentId, port);
    assertConnected(port, currentId);
    results(currentId) = struct('x', time, 'y', value);
end
end

function [y, x] = getSingle(id, port)
data = LabAnalyser.ExReceive(id, port);
if ~isa(data, 'double')
    y = data;
    x = [];
    return;
end

if numel(data) <= 1
    y = data;
    x = [];
    return;
end

half = numel(data) / 2;
x = data(1:half);
y = data(half + 1:end);
end

function assertConnected(port, id)
portPointer = libpointer('cstring', num2str(port));
if ~calllib('TCPClient', 'IsConnected', portPointer)
    error('LabAnalyser:ConnectionLost', ...
        'Connection to LabAnalyser was lost while reading "%s". Reconnect before retrying.', id);
end
end
