% timer function
function tmrfcn(hObject, eventdata, Hfig)

handles = guidata(Hfig);
global x;
global y;
global cc;
global s;


data = parsedata(s);

disp(data);

%got routing info
if data == 0
    return;
end

if data(1) ==1
    [~, l] = size(data);
    n = sqrt(l - 1);

    data = data(2:end);
    data = reshape(data, [n, n]);
    disp(data);
    cla(handles.axes1);
    [x, y] = fixNode(handles, n);

    for i = 1:n
        for j = 1:n
            if i == j
                continue;
            end
            if data(i, j) == 0
                continue;
            else 
                line([x(i), x(j)], [y(i), y(j)], 'color', cc(j, :));
            end
        end
    end
    set(handles.axes1, 'xtick', []);
    set(handles.axes1, 'ytick', []);
end

if data(1)==2
    %got sensor data
    [~, l] = size(data);

    data = data(2:end);
    data = reshape(data, [3, (l - 1)/3]);

    set(handles.gL, 'String', data(1, 1));set(handles.gB, 'String', data(2, 1));
    set(handles.gT, 'String', data(3, 1));

    set(handles.n1L, 'String', data(1, 2));set(handles.n1B, 'String', data(2, 2));
    set(handles.n1T, 'String', data(3, 2));%set(handles.n1R, 'String', data(4, 2));

    set(handles.n2L, 'String', data(1, 3));set(handles.n2B, 'String', data(2, 3));
    set(handles.n2T, 'String', data(3, 3));%set(handles.n2R, 'String', data(4, 3));

    set(handles.n3L, 'String', data(1, 4));set(handles.n3B, 'String', data(2, 4));
    set(handles.n3T, 'String', data(3, 4));%set(handles.n3R, 'String', data(4, 4));

    set(handles.n4L, 'String', data(1, 5));set(handles.n4B, 'String', data(2, 5));
    set(handles.n4T, 'String', data(3, 5));%set(handles.n4R, 'String', data(4, 5));

    %set(handles.n5L, 'String', data(1, 6));set(handles.n5B, 'String', data(2, 6));
    %set(handles.n5T, 'String', data(3, 6));%set(handles.n5R, 'String', data(4, 6));
end

guidata(Hfig, handles);