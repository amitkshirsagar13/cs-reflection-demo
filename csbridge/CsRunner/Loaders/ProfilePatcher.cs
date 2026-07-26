using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using CsRunner.Models;
using Newtonsoft.Json.Linq;
using YamlDotNet.Serialization;

namespace CsRunner.Loaders;

public static class ProfileReflector
{
    /// <summary>
    /// Patches an existing instance dynamically using reflection based on a JSON or YAML patch file.
    /// </summary>
    public static void PatchInstance(UserProfile target, string patchFilePath)
    {
        string ext = Path.GetExtension(patchFilePath).ToLowerInvariant();
        JObject patchData;

        if (ext == ".json")
        {
            patchData = JObject.Parse(File.ReadAllText(patchFilePath));
        }
        else if (ext == ".yml" || ext == ".yaml")
        {
            var deserializer = new DeserializerBuilder().Build();
            using var reader = new StreamReader(patchFilePath);
            var yamlObject = deserializer.Deserialize(reader);
            var jsonString = Newtonsoft.Json.JsonConvert.SerializeObject(yamlObject);
            patchData = JObject.Parse(jsonString);
        }
        else
        {
            throw new NotSupportedException($"Unsupported patch format: {ext}");
        }

        // Use reflection to match and patch top-level properties dynamically
        Type type = typeof(UserProfile);
        foreach (var property in patchData.Properties())
        {
            // Match JSON camelCase properties to C# PascalCase properties
            PropertyInfo? propInfo = type.GetProperty(property.Name, 
                BindingFlags.Public | BindingFlags.Instance | BindingFlags.IgnoreCase);

            if (propInfo == null || !propInfo.CanWrite) continue;

            object? valueToSet = null;
            if (propInfo.PropertyType == typeof(string))
            {
                valueToSet = property.Value.Value<string>() ?? string.Empty;
            }
            else if (propInfo.PropertyType == typeof(int))
            {
                valueToSet = property.Value.Value<int>();
            }
            else if (propInfo.PropertyType == typeof(List<string>))
            {
                var list = propInfo.GetValue(target) as List<string> ?? new List<string>();
                list.Clear();
                if (property.Value is JArray arr)
                {
                    list.AddRange(arr.Values<string>().OfType<string>());
                }
                valueToSet = list;
            }
            else if (propInfo.PropertyType == typeof(Address))
            {
                var addr = propInfo.GetValue(target) as Address ?? new Address();
                if (property.Value is JObject addrObj)
                {
                    foreach (var addrProp in addrObj.Properties())
                    {
                        PropertyInfo? aPropInfo = typeof(Address).GetProperty(addrProp.Name, 
                            BindingFlags.Public | BindingFlags.Instance | BindingFlags.IgnoreCase);
                        if (aPropInfo != null && aPropInfo.CanWrite)
                        {
                            aPropInfo.SetValue(addr, addrProp.Value.Value<string>() ?? string.Empty);
                        }
                    }
                }
                valueToSet = addr;
            }

            // Apply value directly via Reflection instead of explicit setters
            propInfo.SetValue(target, valueToSet);
        }
    }
}