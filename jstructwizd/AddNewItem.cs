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
            try
            {
                _applicationObject = Application as DTE2;

                string template_file_name_src = "";
                string template_file_name_dst = "";

                RegistryKey vs2010 = Registry.CurrentUser.OpenSubKey("software\\jstructtool");

                template_file_name_src = vs2010.GetValue("AppFolder").ToString() + "\\inc\\template.jst";

                vs2010.Close();

                foreach (SelectedItem si in _applicationObject.SelectedItems)
                {
                    VCFilter filter = si.ProjectItem.Object as VCFilter;

                    foreach (VCFile firstFile in filter.Files as IVCCollection)
                    {
                        template_file_name_dst = firstFile.FullPath.Substring(0, firstFile.FullPath.LastIndexOf("\\") + 1) + "template.jst";

                        break;
                    }

                    if ("" == template_file_name_dst)
                    {
                        VCProject proj = filter.project as VCProject;

                        template_file_name_dst = proj.ProjectDirectory + "template.jst";
                    }

                    File.Copy(template_file_name_src, template_file_name_dst);

                    if (filter.CanAddFile(template_file_name_dst)) filter.AddFile(template_file_name_dst);

                    // open added file
                    _applicationObject.ItemOperations.OpenFile(template_file_name_dst);
                }
            }
            catch (Exception e)
            {
            }
        }
    }
}
