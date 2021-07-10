namespace RuntimeSwitcher
{
    partial class MainWindow
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainWindow));
            this.label1 = new System.Windows.Forms.Label();
            this.statusLabel = new System.Windows.Forms.Label();
            this.useOpenComposite = new System.Windows.Forms.Button();
            this.label2 = new System.Windows.Forms.Label();
            this.progressBar = new System.Windows.Forms.ProgressBar();
            this.updatesLabel = new System.Windows.Forms.Label();
            this.doUpdate = new System.Windows.Forms.Button();
            this.useSteamVR = new System.Windows.Forms.Button();
            this.configureApps = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(13, 13);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(40, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Status:";
            // 
            // statusLabel
            // 
            this.statusLabel.AutoSize = true;
            this.statusLabel.Location = new System.Drawing.Point(60, 13);
            this.statusLabel.Name = "statusLabel";
            this.statusLabel.Size = new System.Drawing.Size(45, 13);
            this.statusLabel.TabIndex = 1;
            this.statusLabel.Text = "Loading";
            // 
            // useOpenComposite
            // 
            this.useOpenComposite.Location = new System.Drawing.Point(16, 30);
            this.useOpenComposite.Name = "useOpenComposite";
            this.useOpenComposite.Size = new System.Drawing.Size(253, 23);
            this.useOpenComposite.TabIndex = 2;
            this.useOpenComposite.Text = "Switch to OpenComposite";
            this.useOpenComposite.UseVisualStyleBackColor = true;
            this.useOpenComposite.Click += new System.EventHandler(this.useOpenComposite_Click);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(16, 85);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(334, 13);
            this.label2.TabIndex = 3;
            this.label2.Text = "You can switch back to SteamVR by starting it via Steam\'s VR button";
            // 
            // progressBar
            // 
            this.progressBar.Location = new System.Drawing.Point(16, 101);
            this.progressBar.Name = "progressBar";
            this.progressBar.Size = new System.Drawing.Size(334, 23);
            this.progressBar.TabIndex = 4;
            this.progressBar.Visible = false;
            // 
            // updatesLabel
            // 
            this.updatesLabel.AutoSize = true;
            this.updatesLabel.Location = new System.Drawing.Point(16, 127);
            this.updatesLabel.Name = "updatesLabel";
            this.updatesLabel.Size = new System.Drawing.Size(73, 13);
            this.updatesLabel.TabIndex = 5;
            this.updatesLabel.Text = "Update status";
            this.updatesLabel.Visible = false;
            // 
            // doUpdate
            // 
            this.doUpdate.Location = new System.Drawing.Point(16, 143);
            this.doUpdate.Name = "doUpdate";
            this.doUpdate.Size = new System.Drawing.Size(334, 23);
            this.doUpdate.TabIndex = 6;
            this.doUpdate.Text = "Update Now";
            this.doUpdate.UseVisualStyleBackColor = true;
            this.doUpdate.Visible = false;
            this.doUpdate.Click += new System.EventHandler(this.doUpdate_Click);
            // 
            // useSteamVR
            // 
            this.useSteamVR.Location = new System.Drawing.Point(16, 59);
            this.useSteamVR.Name = "useSteamVR";
            this.useSteamVR.Size = new System.Drawing.Size(334, 23);
            this.useSteamVR.TabIndex = 7;
            this.useSteamVR.Text = "Switch to SteamVR";
            this.useSteamVR.UseVisualStyleBackColor = true;
            this.useSteamVR.Click += new System.EventHandler(this.useSteamVR_Click);
            // 
            // configureApps
            // 
            this.configureApps.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.configureApps.Location = new System.Drawing.Point(275, 30);
            this.configureApps.Name = "configureApps";
            this.configureApps.Size = new System.Drawing.Size(75, 23);
            this.configureApps.TabIndex = 8;
            this.configureApps.Text = "Configure";
            this.configureApps.UseVisualStyleBackColor = true;
            this.configureApps.Click += new System.EventHandler(this.configureApps_Click);
            // 
            // MainWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(369, 183);
            this.Controls.Add(this.configureApps);
            this.Controls.Add(this.useSteamVR);
            this.Controls.Add(this.doUpdate);
            this.Controls.Add(this.updatesLabel);
            this.Controls.Add(this.progressBar);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.useOpenComposite);
            this.Controls.Add(this.statusLabel);
            this.Controls.Add(this.label1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.Name = "MainWindow";
            this.Text = "OpenComposite Runtime Switcher";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label statusLabel;
        private System.Windows.Forms.Button useOpenComposite;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ProgressBar progressBar;
        private System.Windows.Forms.Label updatesLabel;
        private System.Windows.Forms.Button doUpdate;
        private System.Windows.Forms.Button useSteamVR;
        private System.Windows.Forms.Button configureApps;
    }
}

