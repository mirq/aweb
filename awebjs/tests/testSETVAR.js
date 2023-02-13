/* test.js */
print("Testing SetVar operations using WORKBENCH ARexx host");
print("Testing ordinary call");
print("Calling: ARexx.SendCommand(\"WORKBENCH\",\"FAULT 204\")");

ARexx.SendCommand("WORKBENCH","FAULT 204")

for(m in ARexx)
{
    if(m != "SendCommand")
    {
        print(m,"\t",ARexx[m]);
    }
}

print("testing SetVar operations")
print("\n\nTesting call with single VAR");
print("Calling: ARexx.SendCommand(\"WORKBENCH\",\"GETATTR APPLICATION.VERSION VAR wbversion\")");

ARexx.SendCommand("WORKBENCH","GETATTR APPLICATION.VERSION VAR wbversion");

for(m in ARexx)
{
    if(m != "SendCommand")
    {
        print(m,"\t",ARexx[m]);
    }
}

print(ARexx.wbversion)

print("\n\nTesting call with STEM");
print("Calling: ARexx.SendCommand(\"WORKBENCH\",\"GETATTR APPLICATION STEM wbattributes\")");

ARexx.SendCommand("WORKBENCH","GETATTR APPLICATION STEM wbattributes");

for(m in ARexx)
{
    if(m != "SendCommand")
    {
        print(m,"\t",ARexx[m]);
    }
}
print();
for(m in ARexx.wbattributes)
{
     print(m,"\t",ARexx.wbattributes[m]);
}




