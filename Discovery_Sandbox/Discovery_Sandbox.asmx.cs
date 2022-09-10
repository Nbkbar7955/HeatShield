using System;
using System.Collections;
using System.Data.SqlClient;
using System.Web.Services;

namespace Discovery_Sandbox
{
    [WebService(Namespace = "http://WilsonWebsite.com/")]
    [WebServiceBinding(ConformsTo = WsiProfiles.None)]
    [System.ComponentModel.ToolboxItem(false)]
    [System.Web.Script.Services.ScriptService]

    public class SandBox : WebService
    {
        private const string ConnectionString = "Data Source = DAVID-R7\\MSSQLSERVER01;" +
                                       "Integrated Security = True;" +
                                       "Connect Timeout = 15;" +
                                       "Encrypt = False;" +
                                       "TrustServerCertificate = False;" +
                                       "ApplicationIntent = ReadWrite;" +
                                       "MultiSubnetFailover = False";

        [WebMethod(EnableSession = true, MessageName = "endeavor")]

        public string EndeavorPut(string dataStream) => DoInsert(dataStream, ConnectionString);

        private static string DoInsert(string dataStream, string connectStr)
        {
            if (dataStream == null) return "No Input";

            try
            {
                using (var conn = new SqlConnection(connectStr))
                {
                    conn.Open();
                    var insertCommand = new SqlCommand(BuildQueryString(dataStream), conn);
                    insertCommand.ExecuteNonQuery();
                    conn.Close();
                }
            }
            catch (Exception ex)
            {
                return ex.Message;
            }

            return "Success";
        }

        private static string BuildQueryString(string dataStream)
        {
            var inputArr = dataStream.Split(',');
            var nvHash = new Hashtable();

            foreach (var tmp in inputArr)
            {
                var nameValuePairArr = tmp.Split('=');               
                nvHash.Add(nameValuePairArr[0], nameValuePairArr[1]);
            }

            var declarations = "";
            var insertColumns = "(";
            var insertValues = " values (";

            foreach(DictionaryEntry item in nvHash)
            {
                if (declarations == "")
                {
                    declarations = $"declare @{item.Key} varchar(max) set @{item.Key} = '{item.Value}' ";
                }
                else
                {
                    declarations += $"declare @{item.Key} varchar(max) set @{item.Key} = '{item.Value}' ";
                }
                insertColumns += $"{item.Key},";
                insertValues += $"@{item.Key},";
            }
            
            declarations += " INSERT INTO HeatShield.NASA.Discover ";
            insertColumns = String.Format(insertColumns.Substring(0,insertColumns.Length - 1));
            insertValues = String.Format(insertValues.Substring(0,insertValues.Length - 1));
            insertColumns += ") ";
            insertValues += ") ";

            return $@"{declarations}{insertColumns}{insertValues}";
        }



        [WebMethod(EnableSession = true, MessageName = "_QuickTest")]
        public string GetReturnString(string str)
        {
            var tTmp = new Temp();

            return tTmp.GetMyString(str);
        }
    }
}
