function Set(id, value, port)
if nargin < 3
    port = 4080;
end

if isa(value, 'double')
    LabAnalyser.ExSendDouble(id, value, port);
else
    LabAnalyser.ExSendString(id, value, port);
end
end
