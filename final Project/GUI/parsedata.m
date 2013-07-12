function data = parsedata(s)


a = fscanf(s);
disp(a);
if (strfind(a, '##1'))
    a = strrep(a, '##1', '1');
    %disp(a);
    data = sscanf(a, '%f', [1, inf]);
    else if (strfind(a, '##2'))
        a = strrep(a, '##2', '2');
        %disp(a);
        data = sscanf(a, '%f', [1, inf]);
        else data = 0;
        end
end


    
    
