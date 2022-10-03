using System;
using System.Collections;
using System.Data.SqlClient;
using System.Web.Services;

namespace Discovery
{
    [WebService(Namespace = "http://WilsonWebsite.com/")]
    [WebServiceBinding(ConformsTo = WsiProfiles.None)]
    [System.ComponentModel.ToolboxItem(false)]
    [System.Web.Script.Services.ScriptService]


    public class Discovery : WebService
    {
        private const string ConnectionString = "Data Source = DAVID-R7\\MSSQLSERVER01;" +
                               "Integrated Security = True;" +
                               "Connect Timeout = 15;" +
                               "Encrypt = False;" +
                               "TrustServerCertificate = False;" +
                               "ApplicationIntent = ReadWrite;" +
                               "MultiSubnetFailover = False";

        [WebMethod(EnableSession = true, MessageName = "isAlive")]
        public bool IsAlive()
        {
            return true;
        }

        [WebMethod]
        public string HelloWorld()
        {
            return "Hello World";
        }



    }
}
