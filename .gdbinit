python
import sys 
sys.path.insert(0, '/opt/sdk/p6x/opt/ext-toolchain/share/gcc-14.3.1/python')
from libstdcxx.v6.printers import register_libstdcxx_printers
register_libstdcxx_printers (None)
end