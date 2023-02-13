/* testing GetVar operation */

print("Running Test: Please Ensure AWeb is allready started\n");

ARexx.AWEBCFG = new Object();
ARexx.AWEBCFG[0] = 1;
ARexx.AWEBCFG[1] = new Object();
ARexx.AWEBCFG[1].ITEM = "SHOWBUTTONS";
ARexx.AWEBCFG[1].VALUE = "0"

if(ARexx.SendCommand("AWEB.1","SETCFG STEM AWEBCFG"))
{
    
    for(m in ARexx)
    {
        if(m != "SendCommand")
        {
            print(m,"\t",ARexx[m]);
        }
    }
}
else
{
    print("SendCommand() Failed, Host not found?\n");
}


ARexx.AWEBCFG[1].VALUE = "1"


if(ARexx.SendCommand("AWEB.1","SETCFG STEM AWEBCFG"))
{
    
    for(m in ARexx)
    {
        if(m != "SendCommand")
        {
            print(m,"\t",ARexx[m]);
        }
    }
}
else
{
    print("SendCommand() Failed, Host not found?\n");
}

print("Test Complete: AWebs window should have opened and closed without and then with userbuttons");


