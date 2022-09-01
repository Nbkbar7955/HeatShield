using System;
using System.Collections.Generic;
using System.ComponentModel.DataAnnotations.Schema;
using System.Data.SqlClient;
using System.Linq;
using System.Web;
using System.Web.Services;
using Discovery_Sandbox;

namespace Discovery_Sandbox
{
    /// <summary>
    /// Summary description for WebService1
    /// </summary>
    [WebService(Namespace = "http://WilsonWebsite.com/")]
    [WebServiceBinding(ConformsTo = WsiProfiles.None)]
    [System.ComponentModel.ToolboxItem(false)]
    // To allow this Web Service to be called from script, using ASP.NET AJAX, uncomment the following line. 
    [System.Web.Script.Services.ScriptService]
    public class SandBox : System.Web.Services.WebService
    {

        [WebMethod]
    private void DbPut()
        {

            string connectionString = "Data Source = DAVID - R7\\MSSQLSERVER01; Integrated Security = True; Connect Timeout = 30; Encrypt = False; TrustServerCertificate = False; ApplicationIntent = ReadWrite; MultiSubnetFailover = False");

            SqlConnection sql = new SqlConnection(connectionString);

            sql.Open();


            sql.Close();
        }
    
    public string HelloWorld()
        {
            return "Hello World";
        }

        [WebMethod(EnableSession = true)]
    
    public string GetReturnString(string str)
        {
            Temp tTmp = new Temp();

            return tTmp.GetMyString(str);
        }
        




        /*
    [WebMethod(EnableSession = true)]
    // create a new SqlConnection object with the appropriate connection string
    SqlConnection sqlConn = new SqlConnection(connectionString);
    // open the connection
    sqlConn.Open();
    // do some operations ...
    // close the connection
    sqlConn.Close()
        */
    }
}
