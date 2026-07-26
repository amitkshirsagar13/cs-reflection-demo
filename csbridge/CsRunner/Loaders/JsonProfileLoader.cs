using CsRunner.Models;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace CsRunner.Loaders;

/// <summary>Loads user profiles from a JSON file.</summary>
public sealed class JsonProfileLoader : IProfileLoader
{
    public IReadOnlyList<string> SupportedExtensions { get; } = [".json"];

    public IReadOnlyList<UserProfile> Load(string filePath)
    {
        CsLogger.Debug($"JsonProfileLoader.Load: {filePath}");
        var raw = File.ReadAllText(filePath);
        var token = JToken.Parse(raw);

        // Accept either an array at root, or an object with a "users" / "profiles" key
        JArray arr = token switch
        {
            JArray  a                                    => a,
            JObject o when o["users"]    is JArray ua   => ua,
            JObject o when o["profiles"] is JArray pa   => pa,
            _                                            => throw new InvalidDataException(
                                                              "JSON must be an array or object with 'users'/'profiles' array")
        };

        var profiles = new List<UserProfile>();
        foreach (var item in arr)
        {
            try
            {
                var p = MapFromToken(item);
                profiles.Add(p);
                CsLogger.Debug($"  Parsed profile id={p.Id} username={p.Username}");
            }
            catch (Exception ex)
            {
                CsLogger.Warn($"  Skipping item: {ex.Message}");
            }
        }

        CsLogger.Info($"JsonProfileLoader: loaded {profiles.Count} profile(s)");
        return profiles;
    }

    private static UserProfile MapFromToken(JToken t)
    {
        var p = new UserProfile
        {
            Id        = t["id"]?.Value<string>()        ?? string.Empty,
            FirstName = t["firstName"]?.Value<string>() ?? string.Empty,
            LastName  = t["lastName"]?.Value<string>()  ?? string.Empty,
            Username  = t["username"]?.Value<string>()  ?? string.Empty,
            Password  = t["password"]?.Value<string>()  ?? string.Empty,
            Age       = t["age"]?.Value<int>()          ?? 0,
        };

        if (t["emails"] is JArray emails)
            p.Emails.AddRange(emails.Values<string>().OfType<string>());

        if (t["mobiles"] is JArray mobiles)
            p.Mobiles.AddRange(mobiles.Values<string>().OfType<string>());

        if (t["address"] is JObject addr)
        {
            p.Address = new Address
            {
                FirstLine = addr["firstLine"]?.Value<string>() ?? string.Empty,
                AptUnit   = addr["aptUnit"]?.Value<string>()   ?? string.Empty,
                City      = addr["city"]?.Value<string>()      ?? string.Empty,
                State     = addr["state"]?.Value<string>()     ?? string.Empty,
                Zip       = addr["zip"]?.Value<string>()       ?? string.Empty,
            };
        }

        if (string.IsNullOrWhiteSpace(p.Id))
            throw new InvalidDataException("Profile is missing 'id'");

        return p;
    }
}