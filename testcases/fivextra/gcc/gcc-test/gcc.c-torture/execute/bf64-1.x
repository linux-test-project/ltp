# The MCore ABI specifies that bitfields may not exceed 32 bits.
# Hence this tes will fail.

istarget "mcore" && torture_execute_xfaili="*"
