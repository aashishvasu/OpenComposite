namespace RuntimeSwitcher
{
    partial class AppListForm
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
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(AppListForm));
            this.appsList = new System.Windows.Forms.ListView();
            this.contextMenuStrip1 = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.optUseOpenComposite = new System.Windows.Forms.ToolStripMenuItem();
            this.optUseSteamVR = new System.Windows.Forms.ToolStripMenuItem();
            this.optUseDefault = new System.Windows.Forms.ToolStripMenuItem();
            this.optRename = new System.Windows.Forms.ToolStripMenuItem();
            this.label1 = new System.Windows.Forms.Label();
            this.defaultRuntime = new System.Windows.Forms.ComboBox();
            this.contextMenuStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // appsList
            // 
            this.appsList.Alignment = System.Windows.Forms.ListViewAlignment.SnapToGrid;
            this.appsList.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.appsList.LabelEdit = true;
            this.appsList.Location = new System.Drawing.Point(12, 33);
            this.appsList.Name = "appsList";
            this.appsList.Size = new System.Drawing.Size(407, 308);
            this.appsList.TabIndex = 0;
            this.appsList.UseCompatibleStateImageBehavior = false;
            this.appsList.AfterLabelEdit += new System.Windows.Forms.LabelEditEventHandler(this.appsList_AfterLabelEdit);
            this.appsList.KeyDown += new System.Windows.Forms.KeyEventHandler(this.appsList_KeyDown);
            this.appsList.MouseClick += new System.Windows.Forms.MouseEventHandler(this.appsList_MouseClick);
            // 
            // contextMenuStrip1
            // 
            this.contextMenuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.optUseOpenComposite,
            this.optUseSteamVR,
            this.optUseDefault,
            this.optRename});
            this.contextMenuStrip1.Name = "contextMenuStrip1";
            this.contextMenuStrip1.Size = new System.Drawing.Size(223, 92);
            // 
            // optUseOpenComposite
            // 
            this.optUseOpenComposite.Name = "optUseOpenComposite";
            this.optUseOpenComposite.Size = new System.Drawing.Size(222, 22);
            this.optUseOpenComposite.Text = "Always use OpenComposite";
            this.optUseOpenComposite.Click += new System.EventHandler(this.optUseOpenComposite_Click);
            // 
            // optUseSteamVR
            // 
            this.optUseSteamVR.Name = "optUseSteamVR";
            this.optUseSteamVR.Size = new System.Drawing.Size(222, 22);
            this.optUseSteamVR.Text = "Always use SteamVR";
            this.optUseSteamVR.Click += new System.EventHandler(this.optUseSteamVR_Click);
            // 
            // optUseDefault
            // 
            this.optUseDefault.Name = "optUseDefault";
            this.optUseDefault.Size = new System.Drawing.Size(222, 22);
            this.optUseDefault.Text = "Use default runtime";
            this.optUseDefault.Click += new System.EventHandler(this.optUseDefault_Click);
            // 
            // optRename
            // 
            this.optRename.Name = "optRename";
            this.optRename.Size = new System.Drawing.Size(222, 22);
            this.optRename.Text = "Rename";
            this.optRename.Click += new System.EventHandler(this.optRename_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 9);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(83, 13);
            this.label1.TabIndex = 2;
            this.label1.Text = "Default Runtime";
            // 
            // defaultRuntime
            // 
            this.defaultRuntime.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.defaultRuntime.FormattingEnabled = true;
            this.defaultRuntime.Location = new System.Drawing.Point(101, 6);
            this.defaultRuntime.Name = "defaultRuntime";
            this.defaultRuntime.Size = new System.Drawing.Size(121, 21);
            this.defaultRuntime.TabIndex = 3;
            this.defaultRuntime.SelectedIndexChanged += new System.EventHandler(this.defaultRuntime_SelectedIndexChanged);
            // 
            // AppListForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(431, 353);
            this.Controls.Add(this.defaultRuntime);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.appsList);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "AppListForm";
            this.Text = "App Configuration";
            this.contextMenuStrip1.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ListView appsList;
        private System.Windows.Forms.ContextMenuStrip contextMenuStrip1;
        private System.Windows.Forms.ToolStripMenuItem optUseOpenComposite;
        private System.Windows.Forms.ToolStripMenuItem optUseSteamVR;
        private System.Windows.Forms.ToolStripMenuItem optUseDefault;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ComboBox defaultRuntime;
        private System.Windows.Forms.ToolStripMenuItem optRename;
    }
}