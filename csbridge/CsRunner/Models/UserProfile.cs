namespace CsRunner.Models;

/// <summary>Mirrors UserProfileModel::Address in C++.</summary>
public class Address
{
    public string FirstLine { get; set; } = string.Empty;
    public string AptUnit   { get; set; } = string.Empty;
    public string City      { get; set; } = string.Empty;
    public string State     { get; set; } = string.Empty;
    public string Zip       { get; set; } = string.Empty;
}

/// <summary>Mirrors UserProfileModel::UserProfile in C++.</summary>
public class UserProfile
{
    public string         Id        { get; set; } = string.Empty;
    public string         FirstName { get; set; } = string.Empty;
    public string         LastName  { get; set; } = string.Empty;
    public string         Username  { get; set; } = string.Empty;
    public string         Password  { get; set; } = string.Empty;
    public List<string>   Emails    { get; set; } = [];
    public List<string>   Mobiles   { get; set; } = [];
    public Address        Address   { get; set; } = new();
    public int            Age       { get; set; }

    public string FullName => $"{FirstName} {LastName}".Trim();
}