using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using EnvDTE;
using EnvDTE80;
using Microsoft.Win32;
using Microsoft.VisualStudio.VCProjectEngine;

namespace jstructwizd
{
    public class AddNewItem : IDTWizard
    {
        private DTE2 _applicationObject;

        public void Execute(object Application,
            int hwndOwner,
            ref object[] contextParams,
            ref object[] customParams,
            ref EnvDTE.wizardResult retval)
        {
            _applicationObject  = Application as DTE2;

            string template_file_name_src = "";
            string template_file_name_dst = "";

            RegistryKey vs2010 = Registry.CurrentUser.OpenSubKey("SOFTWARE\\Microsoft\\VisualStudio\\10.0_Config");

            if (null != vs2010)
            {
                template_file_name_src = vs2010.GetValue("ShellFolder").ToString() + "VC\\include\\template.h";

                vs2010.Close();
            }

            foreach (SelectedItem si in _applicationObject.SelectedItems)
            {
                VCFilter filter = si.ProjectItem.Object as VCFilter;

                if (null == filter) return;

                foreach (VCFile firstFile in filter.Files as IVCCollection)
                {
                    template_file_name_dst = firstFile.FullPath.Substring(0, firstFile.FullPath.LastIndexOf("\\") + 1) + "template.h";

                    break;
                }

                if ("" == template_file_name_dst)
                {
                    VCProject proj = filter.project as VCProject;

                    if (null == proj) return;

                    template_file_name_dst = proj.ProjectDirectory + "template.h";
                }

                File.Copy(template_file_name_src, template_file_name_dst);

                if (filter.CanAddFile(template_file_name_dst)) filter.AddFile(template_file_name_dst);

                // open added file
            }
        }
    }
}
