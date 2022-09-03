using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Remoting.Messaging;
using System.Web;

namespace Discovery_Sandbox
{
    public class Temp
    {
        public string GetMyString(string myVal)
        {
            if (myVal != null) 
                return $"my val -> RETURN: your val ->{myVal}";
            else
                return "my val -> RETURN: your val -> Nothing";
            
        }
    }
}