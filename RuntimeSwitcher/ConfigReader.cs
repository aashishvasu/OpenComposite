using System;
using System.IO;
using System.Diagnostics;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;

namespace RuntimeSwitcher
{
    class ConfigData
    {
        public IList<string> runtime { get; set; } = new List<string>();

        [JsonExtensionData]
        private IDictionary<string, object> _additionalData;
    }

    class ConfigReader
    {
        private string openvr_dir;
        private string config_file;
        private ConfigData config;

        public IList<string> Runtimes
        {
            get
            {
                return config.runtime;
            }
            set
            {
                config.runtime = value;
                Save();
            }
        }

        public ConfigReader()
        {
            openvr_dir = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData) + "/openvr";

            if(!Directory.Exists(openvr_dir))
            {
                Debug.WriteLine("openvr dir missing, creating");
                Directory.CreateDirectory(openvr_dir);
            }

            config_file = openvr_dir + "/openvrpaths.vrpath";

            if(File.Exists(config_file))
            {
                Debug.WriteLine("reading config file");
                config = JsonConvert.DeserializeObject<ConfigData>(File.ReadAllText(config_file));
            } else
            {
                Debug.WriteLine("no config file, creating");
                config = new ConfigData();
                Save();
            }
        }

        private void Save()
        {
            File.WriteAllText(config_file, JsonConvert.SerializeObject(config, Formatting.Indented));
        }
    }
}
