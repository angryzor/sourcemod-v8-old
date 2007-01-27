using System;
using System.IO;
using System.Diagnostics;

namespace builder
{
	public abstract class ABuilder
	{
		public Config cfg;

		public ABuilder()
		{
		}

		public abstract bool BuildLibrary(Package pkg, Library lib, ref string _binName, ref string _binPath);

		public void UpdateRevisionInfo(Package pkg, Library lib)
		{
			string path = Config.PathFormat("{0}/{1}", cfg.SourceBase, lib.LocalPath);
			string file = Config.PathFormat("{0}/svn_version.h", path);

			if (File.Exists(file))
			{
				UpdateRevisionInfo(path, file);
			}
		}

		public void CopyFolder(Package pkg, string source, string dest, string [] omits)
		{
			string from_base = Config.PathFormat("{0}/{1}", cfg.SourceBase, source);
			string to_base = Config.PathFormat("{0}/{1}/{2}", 
				cfg.OutputBase, 
				pkg.GetBaseFolder(), 
				dest);

			string [] files = Directory.GetFiles(from_base);
			string file;

			for (int i=0; i<files.Length; i++)
			{
				file = Path.GetFileName(files[i]);

				if (omits != null)
				{
					bool skip = false;
					for (int j=0; j<omits.Length; j++)
					{
						if (file.CompareTo(omits[j]) == 0)
						{
							skip = true;
							break;
						}
					}
					if (skip)
					{
						continue;
					}
				}
				dest = Config.PathFormat("{0}/{1}", to_base, file);
				File.Copy(files[i], dest, true);
			}
		}

		public void UpdateRevisionInfo(string path, string file)
		{
			ProcessStartInfo info = new ProcessStartInfo();

			info.WorkingDirectory = path;
			info.FileName = cfg.SVNVersion;
			info.Arguments = "--committed " + path;
			info.UseShellExecute = false;
			info.RedirectStandardOutput = true;

			Process p = Process.Start(info);
			string output = p.StandardOutput.ReadToEnd();
			p.WaitForExit();
			p.Close();

			string [] revs = output.Split(":".ToCharArray(), 2);
			if (revs.Length < 1)
			{
				return;
			}

			string rev = null;
			if (revs.Length == 1)
			{
				rev = revs[0];
			} 
			else 
			{
				rev = revs[1];
			}

			rev = rev.Trim();
			rev = rev.Replace("M", "");
			rev = rev.Replace("S", "");

			string vers = cfg.ProductVersion.Replace(".", ",");

			File.Delete(file);
			StreamWriter sw = File.CreateText(file);

			sw.WriteLine("/** This file is autogenerated by build scripts */");
			sw.WriteLine("");
			sw.WriteLine("#ifndef _INCLUDE_SVN_VERSION_H_");
			sw.WriteLine("#define _INCLUDE_SVN_VERSION_H_");
			sw.WriteLine("");
			sw.WriteLine("#define SVN_REVISION			{0}", rev);
			sw.WriteLine("#define SVN_REVISION_STRING	\"{0}\"", rev);
			sw.WriteLine("#define SVN_FILE_VERSION		{0},{1}", vers, rev);
			sw.WriteLine("");
			sw.WriteLine("#endif //_INCLUDE_SVN_VERSION_H_");
			sw.WriteLine("");

			sw.Close();
		}

		public void BuildPackage(Package pkg)
		{
			string path = Config.PathFormat("{0}/{1}", cfg.OutputBase, pkg.GetBaseFolder());

			if (!Directory.Exists(path))
			{
				Directory.CreateDirectory(path);
			}

			/* Create all dirs */
			string [] paths = pkg.GetFolders();
			for (int i=0; i<paths.GetLength(0); i++)
			{
				path = Config.PathFormat("{0}/{1}/{2}", cfg.OutputBase, pkg.GetBaseFolder(), paths[i]);
				if (!Directory.Exists(path))
				{
					Directory.CreateDirectory(path);
				}
			}

			/* Do primitive copies */
			pkg.OnCopyFolders(this);
			pkg.OnCopyFiles();

			/* Do libraries */
			Library [] libs = pkg.GetLibraries();
			string bin = null, binpath = null;
			for (int i=0; i<libs.Length; i++)
			{
				UpdateRevisionInfo(pkg, libs[i]);
				if (BuildLibrary(pkg, libs[i], ref bin, ref binpath))
				{
					path = Config.PathFormat("{0}/{1}/{2}/{3}",
						cfg.OutputBase,
						pkg.GetBaseFolder(),
						libs[i].Destination,
						bin);
					File.Copy(binpath, path, true);
				}
				else 
				{
					throw new System.Exception("Failed to compile library: " + libs[i].Name);
				}
			}
		}
	}
}
