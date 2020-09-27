"""
    Prints statistics from the numbers of the input file
"""
import numpy
import sys
import re

def keep_number(rr):
    gg = re.search('\d+(\.\d*)?', rr)
    if gg:
        return float(gg.group(0))
    return 0        

with open(sys.argv[1],'r') as f:
    data = list(map(keep_number, f))

to_show = [('Min', 'Avg', 'Max', 'std'),
           (min(data), numpy.average(data), max(data), numpy.std(data))] 

def pretty_show(rows):
    widest_colum = 0
    all_txt = []
    for a_row in rows:
        txt_row = list(map(str, a_row))
        all_txt.append(txt_row)
        widest_colum = max(widest_colum, *map(len, txt_row))

    
    for txt_row in all_txt:
        str_row = '|'.join(map(lambda cc: cc.center(widest_colum),txt_row))
        print(str_row)

pretty_show(to_show)
