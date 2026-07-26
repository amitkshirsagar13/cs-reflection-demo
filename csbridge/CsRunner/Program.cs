using CsRunner;
using CsRunner.Loaders;
using CsRunner.Models;
using Newtonsoft.Json;
using Newtonsoft.Json.Serialization;
using System;
using System.Collections.Generic;
using System.IO;

if (args.Length == 0)
{
    CsLogger.Error("Usage: CsRunner <file_path> [existing_user_state_file]");
    return 1;
}

string filePath = args[0];
CsLogger.Info($"CsRunner starting. File: {filePath}");

if (!File.Exists(filePath))
{
    CsLogger.Error($"File not found: {filePath}");
    return 2;
}

try
{
// ════════════ PLACE DEBUG INTERCEPT HERE ════════════
#if DEBUG
    // Triggers if you set the environment variable CSRUNNER_DEBUG=1 in launch.json
    if (Environment.GetEnvironmentVariable("CSRUNNER_DEBUG") == "1") 
    {
        Console.Error.WriteLine($"[DEBUG] Process ID: {Environment.ProcessId}. Waiting for debugger...");
        while (!System.Diagnostics.Debugger.IsAttached)
        {
            Thread.Sleep(100);
        }
    }
#endif
    var settings = new JsonSerializerSettings
    {
        ContractResolver  = new CamelCasePropertyNamesContractResolver(),
        Formatting        = Formatting.None,
        NullValueHandling = NullValueHandling.Ignore,
    };

    List<UserProfile> profilesToOutput;

    // Determine if we are performing a patch operation by verifying the secondary state file argument
    bool isPatchOp = args.Length > 1 && !string.IsNullOrWhiteSpace(args[1]) && File.Exists(args[1]);

    if (isPatchOp)
    {
        CsLogger.Info("Routing invocation to reflection patch engine using existing state file...");
        
        // Read the baseline state exported by C++
        string existingJson = File.ReadAllText(args[1]);
        var targetProfile = JsonConvert.DeserializeObject<UserProfile>(existingJson);
        
        if (targetProfile == null)
        {
            CsLogger.Error("Failed to deserialize the target profile structure from the temporary state file.");
            return 4;
        }

        // Apply the incoming patch fields via Reflection directly onto the populated instance
        ProfileReflector.PatchInstance(targetProfile, filePath);
        
        profilesToOutput = new List<UserProfile> { targetProfile };
    }
    else
    {
        // Default bulk loading flow
        var registry = new LoaderRegistry();
        var loadedProfiles = registry.Load(filePath);
        CsLogger.Info($"Total profiles loaded: {loadedProfiles.Count}");
        
        profilesToOutput = new List<UserProfile>(loadedProfiles);
    }

    string json = JsonConvert.SerializeObject(profilesToOutput, settings);

    Console.WriteLine("---PROFILES_JSON_START---");
    Console.WriteLine(json);
    Console.WriteLine("---PROFILES_JSON_END---");

    return 0;
}
catch (Exception ex)
{
    CsLogger.Error($"Unhandled exception: {ex}");
    return 99;
}