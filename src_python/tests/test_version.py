import pytest
import os
import re

def test_version():
    import happyserial
    
    # Python
    pyversion = happyserial.HappyVersion.HAPPYVERSION
    print('pyversion:   {}'.format(pyversion))
    
    # c
    cversion  = None
    with open(os.path.join('..','src_c','happyserial','happyversion.h'),'r') as f:
        lines = f.readlines()
        for line in lines:
            # format of the line: "#define HAPPYVERSION {1,1,0}"
            m = re.match('#define HAPPYVERSION {([0-9]*),([0-9]*),([0-9]*)}', line)
            if m:
                cversion = (int(m[1]),int(m[2]),int(m[3]))
                break
    print('cversion:    {}'.format(cversion))
    
    # compare
    assert pyversion==cversion
