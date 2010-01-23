using System;
using Gtk;
using System.Threading;
public partial class MainWindow: Gtk.Window
{	
	int mt5Number = 0;
	static double progress;
	protected static String yToolsBasePath = "./";
	public MainWindow (): base (Gtk.WindowType.Toplevel)
	{
		Build ();
		Application.Init();
		Application.Run ();
	}
	
	protected void OnDeleteEvent (object sender, DeleteEventArgs a)
	{
		Application.Quit ();
		a.RetVal = true;
	}
	
	protected static void exec(String program, String arguments)
	{
		/*Gtk.Application.Invoke (delegate {
			textview1.Buffer.Text += program + " " + arguments + "\n";});*/
		System.Diagnostics.Process p = new System.Diagnostics.Process();
		p.StartInfo = new System.Diagnostics.ProcessStartInfo(program, arguments);
		p.StartInfo.RedirectStandardOutput = true;
		p.StartInfo.UseShellExecute = false;
		p.Start();
		while ( ! p.HasExited )
		{}
			/*Gtk.Application.Invoke (delegate {
		    textview1.Buffer.Text += p.StandardOutput.ReadToEnd();});*/
	}
	protected static void createMt5File(String mt5Path, String outputBasePath, Boolean withoutStrips)
	{
		String strips = "";
		if(withoutStrips) strips = "--no-strips";
		String[] slashSplitMt5Path = mt5Path.Split(System.IO.Path.DirectorySeparatorChar);
		String[] dotSplitMt5File = 
			slashSplitMt5Path[slashSplitMt5Path.Length-1].Split('.');
		String objfName = dotSplitMt5File[0] + (strips.Equals("")?".objf":".obj");
		String mtlName = dotSplitMt5File[0] + ".mtl";
		String textureNamePrefix = dotSplitMt5File[0] + ".mtl";
		String ypvrArgs = "\"" + mt5Path + "\" \"" 
			+ outputBasePath + "/"
				+ textureNamePrefix + "\"";
		String ymt5Args = strips + " \"" + mt5Path + "\" \""
			+ outputBasePath + "\" " + objfName + " " + mtlName + " " 
				+ textureNamePrefix + "";
		ypvrArgs = String.Join("/", ypvrArgs.Split(System.IO.Path.DirectorySeparatorChar));
		ymt5Args = String.Join("/",ymt5Args.Split(System.IO.Path.DirectorySeparatorChar));
		exec(yToolsBasePath + "ypvr", ypvrArgs);
		exec(yToolsBasePath + "ymt5", ymt5Args);
		Gtk.Application.Invoke (delegate {
		progressbar1.Fraction += progress;
		});
	}
	protected virtual void recursiveCountMt5Files(String mt5Path)
	{
		if(System.IO.Directory.Exists(mt5Path))
		{
			String[] files = System.IO.Directory.GetFiles(mt5Path);
			foreach(String file in files)
			{
				recursiveCountMt5Files(mt5Path + System.IO.Path.DirectorySeparatorChar + System.IO.Path.GetFileName(file));
			}
			String[] directories = System.IO.Directory.GetDirectories(mt5Path);
			foreach(String file in directories)
			{
				recursiveCountMt5Files(mt5Path + System.IO.Path.DirectorySeparatorChar + System.IO.Path.GetFileName(file));
			}
		}
		else if(System.IO.File.Exists(mt5Path) && 
		        System.IO.Path.GetExtension(mt5Path).ToUpper()==".MT5")
					this.mt5Number++;
	}
	protected static void recursiveCreateMt5File(String mt5Path, String outputBasePath, Boolean withoutStrips)
	{
		if(System.IO.Directory.Exists(mt5Path))
		{
			String currentDirectory = System.IO.Path.GetFileName(mt5Path);
			String newOutputBasePath = outputBasePath + System.IO.Path.DirectorySeparatorChar + currentDirectory;
			System.IO.Directory.CreateDirectory(newOutputBasePath);
			String[] files = System.IO.Directory.GetFiles(mt5Path);
			foreach(String file in files)
			{
				recursiveCreateMt5File(mt5Path + System.IO.Path.DirectorySeparatorChar + System.IO.Path.GetFileName(file), newOutputBasePath, withoutStrips);
			}
			String[] directories = System.IO.Directory.GetDirectories(mt5Path);
			foreach(String file in directories)
			{
				recursiveCreateMt5File(mt5Path + System.IO.Path.DirectorySeparatorChar + System.IO.Path.GetFileName(file), newOutputBasePath, withoutStrips);
			}
		}
		else if(System.IO.File.Exists(mt5Path)
		        && System.IO.Path.GetExtension(mt5Path).ToUpper()==".MT5")
		{
			Gtk.Application.Invoke (delegate {
			statusbar1.Push(1,"extracting " + mt5Path);
			});
			createMt5File(mt5Path, outputBasePath, withoutStrips);
		}
	}
	protected virtual void OnButton2Clicked (object sender, System.EventArgs e)
	{
		String mt5Path = filechooserbutton1.Filename;
		String outputBasePath = filechooserbutton2.CurrentFolder;
		Boolean withoutStrips = checkbutton1.Active;
		if(mt5Path == "")
		{
			statusbar1.Push(1,"error : \"" + mt5Path + "\" Wrong MT5 File Path");
			return;
		}
		if(outputBasePath == "" || !System.IO.Directory.Exists(outputBasePath))
		{
			statusbar1.Push(1,"error : Wrong Output Directory");
			return;
		}
		button2.Sensitive = false;
		this.mt5Number = 0;
		recursiveCountMt5Files(mt5Path);
		progress = 1.0 / this.mt5Number;
		Thread thr = new Thread (mt5BaseFunction);
		object[] array = new object[3];
		array[0] = mt5Path;
		array[1] = outputBasePath;
		array[2] = withoutStrips;
    	thr.Start (array);
	}
	protected static void mt5BaseFunction(object data)
	{
		object[] array = (object[]) data;
		recursiveCreateMt5File((String)array[0], (String) array[1], (Boolean) array[2]);
		Gtk.Application.Invoke (delegate {
		progressbar1.Fraction = 1;
		button2.Sensitive = true;
		statusbar1.Push(1,"done");
		});
	}

	protected virtual void onMt5DirectoryToggle (object sender, System.EventArgs e)
	{
		if(checkbutton2.Active)
			this.filechooserbutton1.Action = (Gtk.FileChooserAction)(2);
		else
			this.filechooserbutton1.Action = (Gtk.FileChooserAction)(0);
	}
}