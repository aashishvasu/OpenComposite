using Newtonsoft.Json;
using Newtonsoft.Json.Converters;
using Newtonsoft.Json.Linq;
using OSIcon;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace RuntimeSwitcher
{
    public partial class AppListForm : Form
    {
        public List<VRApp> Apps { get; } = new List<VRApp>();

        private readonly Dictionary<VRApp, ListViewItem> _items = new Dictionary<VRApp, ListViewItem>();
        private readonly Dictionary<ListViewItem, VRApp> _itemApps = new Dictionary<ListViewItem, VRApp>();

        private readonly ListViewGroup _groupOpenComposite, _groupSteamVr;

        private readonly string _configPath;
        private readonly Dictionary<string, JObject> _config;
        private readonly JObject _globalConfig;

        private const string LblOpenComposite = "OpenComposite";
        private const string LblSteamVr = "SteamVR";
        private const string KeyGlobalConfig = "***GlobalConfig";
        private const string KeyDefaultRuntime = "default_runtime";

        public AppListForm(string appListPath, string configPath)
        {
            InitializeComponent();

            this._configPath = configPath;
            _config = ReadAppListFile(configPath);
            Dictionary<string, JObject> map = ReadAppListFile(appListPath);

            foreach (KeyValuePair<string, JObject> item in map)
            {
                if (!_config.ContainsKey(item.Key))
                {
                    _config[item.Key] = new JObject();
                }

                Apps.Add(new VRApp(item.Key, item.Value, _config[item.Key]));
            }

            _groupOpenComposite = new ListViewGroup("OpenComposite", HorizontalAlignment.Left);
            _groupSteamVr = new ListViewGroup("SteamVR", HorizontalAlignment.Left);

            appsList.Groups.AddRange(new[] {_groupOpenComposite, _groupSteamVr});

            // Get the global config data
            if (!_config.ContainsKey(KeyGlobalConfig))
                _config[KeyGlobalConfig] = new JObject();
            _globalConfig = _config[KeyGlobalConfig];

            if (!_globalConfig.ContainsKey(KeyDefaultRuntime))
                _globalConfig[KeyDefaultRuntime] = JToken.FromObject(VRApp.Runtime.OpenComposite);

            ////

            // Create two ImageList objects.
            ImageList imageListSmall = new ImageList
            {
                ImageSize = new Size(32, 32),
                TransparentColor = Color.Transparent,
                ColorDepth = ColorDepth.Depth32Bit,
            };

            ImageList imageListLarge = new ImageList
            {
                ImageSize = new Size(48, 48),
                TransparentColor = Color.Transparent,
                ColorDepth = ColorDepth.Depth32Bit,
            };

            foreach (VRApp app in Apps)
            {
                int imageId = imageListSmall.Images.Count;
                imageListSmall.Images.Add(app.GetHighQualityIcon(IconSize.Large));
                imageListLarge.Images.Add(app.GetHighQualityIcon(IconSize.ExtraLarge));

                ListViewItem item = new ListViewItem(app.Name, imageId);
                appsList.Items.Add(item);

                _items[app] = item;
                _itemApps[item] = app;

                UpdateViewFor(app);
            }

            //Assign the ImageList objects to the ListView.
            appsList.LargeImageList = imageListLarge;
            appsList.SmallImageList = imageListSmall;

            // Set up the defaults selector
            defaultRuntime.Items.Clear();
            defaultRuntime.Items.AddRange(new object[] {LblOpenComposite, LblSteamVr});
            defaultRuntime.SelectedIndex =
                _globalConfig[KeyDefaultRuntime].ToObject<VRApp.Runtime>() == VRApp.Runtime.SteamVR ? 1 : 0;
        }

        private void Save()
        {
            File.WriteAllText(_configPath, JsonConvert.SerializeObject(_config, Formatting.Indented));
        }

        private void UpdateViewFor(VRApp app)
        {
            ListViewItem item = _items[app];
            item.Text = app.Name;
            switch (app.SelectedRuntime)
            {
                case VRApp.Runtime.Default:
                    item.Group = null;
                    break;
                case VRApp.Runtime.OpenComposite:
                    item.Group = _groupOpenComposite;
                    break;
                case VRApp.Runtime.SteamVR:
                    item.Group = _groupSteamVr;
                    break;
            }
        }

        private static Dictionary<string, JObject> ReadAppListFile(string path)
        {
            if (!File.Exists(path))
                return new Dictionary<string, JObject>();

            return JsonConvert.DeserializeObject<Dictionary<string, JObject>>(File.ReadAllText(path));
        }

        private void appsList_MouseClick(object sender, MouseEventArgs e)
        {
            if (e.Button != MouseButtons.Right)
                return;

            if (!appsList.FocusedItem.Bounds.Contains(e.Location))
                return;

            contextMenuStrip1.Show(Cursor.Position);
        }

        private void SetAppsMode(VRApp.Runtime runtime)
        {
            foreach (ListViewItem item in appsList.SelectedItems)
            {
                VRApp app = _itemApps[item];
                app.SelectedRuntime = runtime;
                UpdateViewFor(app);
            }

            Save();
        }

        private void optUseOpenComposite_Click(object sender, EventArgs e)
        {
            SetAppsMode(VRApp.Runtime.OpenComposite);
        }

        private void optUseSteamVR_Click(object sender, EventArgs e)
        {
            SetAppsMode(VRApp.Runtime.SteamVR);
        }

        private void optUseDefault_Click(object sender, EventArgs e)
        {
            SetAppsMode(VRApp.Runtime.Default);
        }

        private void defaultRuntime_SelectedIndexChanged(object sender, EventArgs e)
        {
            _globalConfig[KeyDefaultRuntime] = JToken.FromObject(defaultRuntime.SelectedIndex == 1
                ? VRApp.Runtime.SteamVR
                : VRApp.Runtime.OpenComposite);
            Save();
        }

        private void appsList_AfterLabelEdit(object sender, LabelEditEventArgs e)
        {
            VRApp app = _itemApps[appsList.Items[e.Item]];
            string newName = e.Label;

            if (newName == null)
                return;

            e.CancelEdit = true;
            app.Name = newName == "" ? null : newName;
            UpdateViewFor(app);
            Save();
        }

        private void appsList_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyData != Keys.F2) return;

            optRename_Click(null, null);
        }

        private void optRename_Click(object sender, EventArgs e)
        {
            if (appsList.SelectedItems.Count <= 0)
                return;

            appsList.SelectedItems[0].BeginEdit();
        }

        public class VRApp
        {
            public string Path { get; }
            public DateTime Timestamp { get; }

            public string FileName => Path.Substring(Path.LastIndexOfAny(new[] {'/', '\\'}) + 1);

            public string Name
            {
                get => _config.ContainsKey("name")
                    ? _config["name"].ToString()
                    : FileName.Substring(0, FileName.LastIndexOf('.'));
                set
                {
                    if (value == null)
                        _config.Remove("name");
                    else
                        _config["name"] = JValue.CreateString(value);
                }
            }

            private readonly JObject _config;

            public Runtime SelectedRuntime
            {
                get => _config.ContainsKey("runtime") ? _config["runtime"].ToObject<Runtime>() : Runtime.Default;
                set
                {
                    if (value == Runtime.Default)
                        _config.Remove("runtime");
                    else
                        _config["runtime"] = JToken.FromObject(value);
                }
            }

            public VRApp(string exeName, JObject data, JObject config)
            {
                Path = exeName;
                _config = config;

                long timestamp = data["timestamp"].ToObject<long>();
                Timestamp = DateTimeOffset.FromUnixTimeMilliseconds(timestamp).DateTime;
            }

            public Icon GetHighQualityIcon(IconSize size)
            {
                IconInfo info = IconReader.ExtractIconFromFileEx(Path, size);
                return info.Icon;
            }

            public enum Runtime
            {
                Default,
                OpenComposite,
                SteamVR,
            }
        }
    }
}
