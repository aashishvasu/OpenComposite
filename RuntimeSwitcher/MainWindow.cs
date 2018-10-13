using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace RuntimeSwitcher
{
    public partial class MainWindow : Form
    {
        private ConfigReader config;
        private string ocRuntimePath;
        private string ocBinPath;

        public MainWindow()
        {
            ocRuntimePath = Directory.GetParent(System.Reflection.Assembly.GetEntryAssembly().Location).FullName;
            ocRuntimePath += Path.DirectorySeparatorChar + "Runtime";
            ocBinPath = ocRuntimePath + Path.DirectorySeparatorChar + "bin";

            if (!Directory.Exists(ocRuntimePath)) {
                Directory.CreateDirectory(ocRuntimePath);
            }

            if (!Directory.Exists(ocBinPath))
            {
                Directory.CreateDirectory(ocBinPath);
            }

            config = new ConfigReader();

            InitializeComponent();

            UpdateStatus();
        }

        private void UpdateStatus()
        {
            string runtime = config.Runtimes.Count == 0 ? null : config.Runtimes.First();
            useOpenComposite.Enabled = true;

            if (runtime == null)
            {
                statusLabel.Text = "None";
            }
            else if (runtime.Contains("SteamVR"))
            {
                statusLabel.Text = "SteamVR";
            }
            else if (runtime == ocRuntimePath)
            {
                statusLabel.Text = "OpenComposite";
                useOpenComposite.Enabled = false;
            }
            else
            {
                statusLabel.Text = "Other";
            }
        }

        private void SwitchToOpenComposite()
        {
            List<string> rts = new List<string>(config.Runtimes);
            rts.RemoveAll(s => s == ocRuntimePath);
            rts.Insert(0, ocRuntimePath);
            config.Runtimes = rts;

            UpdateStatus();
        }

        private async Task<bool> UpdateDLLs()
        {

            bool downloads = false;

            string vrclient = ocBinPath + Path.DirectorySeparatorChar + "vrclient.dll";
            if (!File.Exists(vrclient))
            {
                downloads = true;
                statusLabel.Text = "Downloading 32-bit DLL";
                await DownloadFile(vrclient, "https://znix.xyz/OpenComposite/download.php?arch=x86");
            }

            string vrclient_x64 = ocBinPath + Path.DirectorySeparatorChar + "vrclient_x64.dll";
            if (!File.Exists(vrclient_x64))
            {
                downloads = true;
                statusLabel.Text = "Downloading 64-bit DLL";
                await DownloadFile(vrclient_x64, "https://znix.xyz/OpenComposite/download.php?arch=x64");
            }

            return downloads;
        }

        private async void useOpenComposite_Click(object sender, EventArgs e)
        {
            useOpenComposite.Enabled = false;

            bool downloads = await UpdateDLLs();

            SwitchToOpenComposite();

            if(downloads)
            {
                statusLabel.Text += " (Download Complete)";
            }
        }

        private async Task DownloadFile(string name, string url)
        {
            using (WebClient wc = new WebClient())
            {
                progressBar.Visible = true;

                wc.DownloadProgressChanged += (object sender, DownloadProgressChangedEventArgs e) =>
                {
                    progressBar.Value = e.ProgressPercentage;
                };

                // Ensure the file gets redownloaded if the program is closed during download
                string temp = name + ".part";

                await wc.DownloadFileTaskAsync(
                    // Param1 = Link of file
                    new System.Uri(url),
                    // Param2 = Path to save
                    temp
                );

                File.Move(temp, name);

                progressBar.Value = 0;
                progressBar.Visible = false;
            }
        }
    }
}
