using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace RuntimeSwitcher
{
    public partial class MainWindow : Form
    {
        private static readonly Func<string, bool> STEAMVR_TEST = s => s.Contains("SteamVR");

        private ConfigReader config;
        private string ocRuntimePath;
        private string ocBinPath;
        private string revisionFilePath;
        private string appListPath;
        private string appConfigPath;

        private SemaphoreSlim dllDownloadMutex = new SemaphoreSlim(1);

        private AppListForm appList;

        public MainWindow()
        {
            ocRuntimePath = Directory.GetParent(System.Reflection.Assembly.GetEntryAssembly().Location).FullName;
            ocRuntimePath += Path.DirectorySeparatorChar + "Runtime";
            ocBinPath = ocRuntimePath + Path.DirectorySeparatorChar + "bin";
            revisionFilePath = ocRuntimePath + Path.DirectorySeparatorChar + "revision.txt";
            appListPath = ocBinPath + Path.DirectorySeparatorChar + "applist.json";
            appConfigPath = ocBinPath + Path.DirectorySeparatorChar + "apps-config.json";

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

            if(File.Exists(revisionFilePath))
            {
                CheckForUpdate();
            }
        }

        private void UpdateStatus()
        {
            string runtime = config.Runtimes.Count == 0 ? null : config.Runtimes.First();
            useOpenComposite.Enabled = true;

            useSteamVR.Enabled = config.Runtimes.Where(STEAMVR_TEST).Count() > 0;

            if (runtime == null)
            {
                statusLabel.Text = "None";
            }
            else if (STEAMVR_TEST.Invoke(runtime))
            {
                statusLabel.Text = "SteamVR";
                useSteamVR.Enabled = false;
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

        private async void CheckForUpdate()
        {
            string currentRevision = File.ReadAllText(revisionFilePath).Trim();

            updatesLabel.Visible = true;
            updatesLabel.Text = "Checking for updates...";

            string serverRevision = await UpdateChecker.GetLatestHash();

            if(serverRevision != currentRevision)
            {
                updatesLabel.Text = "Update Found!";
                doUpdate.Visible = true;
            } else
            {
                updatesLabel.Text = "Already up-to-date";
            }
        }

        private async void doUpdate_Click(object sender, EventArgs e)
        {
            doUpdate.Enabled = false;
            updatesLabel.Text = "Updating...";

            File.Delete(ocBinPath + Path.DirectorySeparatorChar + "vrclient.dll");
            File.Delete(ocBinPath + Path.DirectorySeparatorChar + "vrclient_x64.dll");

            await UpdateDLLs();

            doUpdate.Visible = false;
            updatesLabel.Text = "Update Complete";
        }

        private void SwitchToOpenComposite()
        {
            List<string> rts = new List<string>(config.Runtimes);
            rts.RemoveAll(s => s == ocRuntimePath);
            rts.Insert(0, ocRuntimePath);
            config.Runtimes = rts;

            UpdateStatus();

            // Copy the 'version.txt' file from SteamVR's bin directory to OpenComposite's bin directory
            //  this stops the Steam client from automatically changing this
            //  credit to ArtyDidNothingWrong for finding this:
            //  see https://www.reddit.com/r/oculus/comments/9nxixe/systemwide_installation_for_opencomposite_released/e7qg27q/

            // Check SteamVR is installed, otherwise we don't need to bother with this
            string svrPath = rts.FirstOrDefault(STEAMVR_TEST);
            if (string.IsNullOrEmpty(svrPath))
                return;

            string svrVersionFile = Path.Combine(svrPath, "bin", "version.txt");
            string ocVersionFile = Path.Combine(ocBinPath, "version.txt");

            // If the SteamVR version file is missing (eg a corrupted SteamVR installation), stop here.
            if(!File.Exists(svrVersionFile))
                return;

            File.Copy(svrVersionFile, ocVersionFile, true);
        }

        private async Task<bool> UpdateDLLs()
        {
            await dllDownloadMutex.WaitAsync().ConfigureAwait(false);

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

            File.WriteAllText(revisionFilePath, await UpdateChecker.GetLatestHash());

            dllDownloadMutex.Release();

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

        private void useSteamVR_Click(object sender, EventArgs e)
        {
            List<string> rts = new List<string>(config.Runtimes);

            // Remove OpenComposite
            rts.RemoveAll(s => s == ocRuntimePath);

            // Move SteamVR to the top of the list
            string svrPath = rts.FirstOrDefault(STEAMVR_TEST);
            if (!string.IsNullOrEmpty(svrPath))
            {
                rts.RemoveAll(s => s == svrPath);
                rts.Insert(0, svrPath);
            }

            // Apply the changes and update the UI
            config.Runtimes = rts;
            UpdateStatus();
        }

        private void configureApps_Click(object sender, EventArgs e)
        {
            if (appList != null && !appList.IsDisposed)
            {
                appList.Focus();
                return;
            }

            appList = new AppListForm(appListPath, appConfigPath);
            appList.Show(this);
        }
    }
}
