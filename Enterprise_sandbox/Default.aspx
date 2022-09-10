<%@ Page Title="Home Page" Language="C#" MasterPageFile="~/Site.Master" AutoEventWireup="true" CodeBehind="Default.aspx.cs" Inherits="Enterprise_sandbox._Default" %>

<asp:Content ID="BodyContent" ContentPlaceHolderID="MainContent" runat="server">

    <div class="jumbotron">
        <h1>HeatShield</h1>
        <p class="lead">HeatShield is the project to manage the Hydronic Heating system in our house</p>
        <p>Details eventually. Or maybe not...</p>
    </div>

    <div class="row">
        <asp:DataList ID="DataList1" runat="server" DataSourceID="DiscoveryDatabase">
            <ItemTemplate>
                Expr1:
                <asp:Label ID="Expr1Label" runat="server" Text='<%# Eval("Expr1") %>' />
                <br />
                Expr2:
                <asp:Label ID="Expr2Label" runat="server" Text='<%# Eval("Expr2") %>' />
                <br />
                version_number:
                <asp:Label ID="version_numberLabel" runat="server" Text='<%# Eval("version_number") %>' />
                <br />
                config:
                <asp:Label ID="configLabel" runat="server" Text='<%# Eval("config") %>' />
                <br />
<br />
            </ItemTemplate>
        </asp:DataList>
        

        <asp:SqlDataSource ID="DiscoveryDatabase" runat="server" ConnectionString="<%$ ConnectionStrings:HeatShieldConnectionString %>" SelectCommand="SELECT version_number AS Expr1, config AS Expr2, NASA.Discover.* FROM NASA.Discover"></asp:SqlDataSource>
        <br />
        <asp:GridView ID="GridView1" runat="server" AutoGenerateColumns="False" DataSourceID="DiscoveryDatabase">
            <Columns>
                <asp:BoundField DataField="Expr1" HeaderText="Expr1" SortExpression="Expr1" />
                <asp:BoundField DataField="Expr2" HeaderText="Expr2" SortExpression="Expr2" />
                <asp:BoundField DataField="version_number" HeaderText="version_number" SortExpression="version_number" />
                <asp:BoundField DataField="config" HeaderText="config" SortExpression="config" />
            </Columns>
        </asp:GridView>
        <br />
    </div>

</asp:Content>
