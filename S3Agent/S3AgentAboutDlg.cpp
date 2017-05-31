// CAboutDlg dialog used for App About
#include "stdafx.h"
#include "S3AgentAboutDlg.h"


CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{

}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_ABOUT_TEXT, m_AboutText);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

BOOL CAboutDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    CString AboutText;

    //Remember to update the information in Resource.rc as well as here!
    AboutText = "Sentinel 3 Remote Agent\r\n"
        "Version: 1.1 for Sentinel 3 SW\r\n"
        "Copyright (C) Pulse Power and Measurement Ltd 2016. All rights reserved.\r\n"
        "\r\nDistributed under the following License:"
        "\r\n\r\n"
       "PPM Proprietary Software License\r\n\
BY USING THE SOFTWARE, YOU ACCEPT THESE TERMS. IF YOU DO NOT ACCEPT THEM, DO NOT USE THE SOFTWARE.\r\n\r\n\
Terms and Conditions\r\n\
\r\n\
\
0.Definitions:\r\n\
a.“License” refers to the terms and conditions of use, reproduction, and distributions, as defined by sections 0 through 12 of this document.\r\n\
b.“The Software” refers to any copyrightable work licensed under this License (including all related documentation and Upgrades that may be provided by The Company at its discretion). The Company may provide certain third party software subject to separate license terms either presented at the time of installation or otherwise provided with the Software (\"Third Party Software\"). Such Third Party Software is not included in the definition of the term \"Software\". Any Third Party Software, linked to or referenced from the Software, are licensed to you by the third parties that own such code, not by the Company.\r\n\
c.“The Company” shall mean “Pulse Power and Measurement Limited”\r\n\
d.“The Licensee” means you.\r\n\
\r\n\
1.Permissions\r\n\
a.The Software is licensed to the Licensee under the terms and conditions of this License, and is not sold to the Licensee.\r\n\
b.All applicable intellectual property rights, copyrights, and other rights not expressly granted in license are reserved by the Company.\r\n\
c.This License is granted perpetually, as long as the Licensee does not materially breach it.\r\n\
d.The Licensee may install and use any number of copies of the software on the Licensee’s devices. \r\n\
\r\n\
2.Acceptance\r\n\
a.By downloading the software, completing the installation process, and/or using the Software, you consent to the terms of this License and you agree to be bound by this License. \r\n\
b.If you do not wish to become a party to this License and be bound by all of its terms and conditions, cancel the installation process, do not install or use the software, and immediately delete all of the software and any applicable documentation.\r\n\
\r\n\
3.Redistributions\r\n\
a.The license to the Software granted herein is non-transferable and Licensee may not, without the prior written consent of The Company, distribute or otherwise provide the Software to any third party except as part of a Licensee Product as permitted by this Agreement.\r\n\
\r\n\
4.Updates, Upgrades and Fixes\r\n\
a.The Company may provide the Licensee from time to time with Upgrades, Updates or Fixes, as detailed herein and according to his sole discretion. The Licensee warrants to keep The Software up-to-date and install all relevant updates and fixes, and may, at his sole discretion, purchase upgrades, according to the rates set by The Company. \r\n\
b.The Company shall provide any update or Fix free of charge; however, nothing in this License shall require The Company to provide Updates or Fixes. \r\n\
c.The Company may require remuneration for Upgrades to the Software; however, nothing in this License shall require The Company to provide Upgrades.\r\n\
d.Upgrades: \r\n\
i.For the purpose of this license, an Upgrade shall be a material amendment in The Software, which contains new features and or major performance improvements and shall be marked as a new version number. For example, should Licensee purchase The Software under version 1.X.X, an upgrade shall commence under number 2.0.0.\r\n\
e.Updates:\r\n\
i.For the purpose of this license, an update shall be a minor amendment in The Software, which may contain new features or minor improvements and shall be marked as a new sub-version number. For example, should Licensee purchase The Software under version 1.1.X, an upgrade shall commence under number 1.2.0.\r\n\
f.Fix:\r\n\
i.For the purpose of this license, a fix shall be a minor amendment in The Software, intended to remove bugs or alter minor features which impair the Software's functionality. A fix shall be marked as a new sub-sub-version number. For example, should Licensee purchase Software under version 1.1.1, an upgrade shall commence under number 1.1.2.\r\n\
\r\n\
5.Support\r\n\
a.The Software is provided under an AS-IS basis and without any support, updates or maintenance. Nothing in this License shall require The Company to provide the Licensee with support or fixes to any bug, failure, mis-performance or other defect in The Software.\r\n\
b.Bug Notification\r\n\
i.The Licensee may provide The Company of details regarding any bug, defect or failure in The Software promptly and with no delay from such event\r\n\
ii.The Licensee shall comply with The Company’s request for information regarding bugs, defects or failures and furnish him with information, screenshots and try to reproduce such bugs, defects or failures.\r\n\
c.Feature Request: \r\n\
i.The Licensee may request additional features in Software, provided, however, that \r\n\
1.The Licensee shall waive any claim or right in such feature should feature be developed by The Company.\r\n\
2.The Licensee shall be prohibited from developing the feature, or disclose such feature request, or feature, to any 3rd party directly competing with The Company or any 3rd party which may be, following the development of such feature, in direct competition with The Company.\r\n\
3.The Licensee warrants that feature does not infringe any 3rd party patent, trademark, trade-secret or any other intellectual property right\r\n\
4.The Licensee developed, envisioned or created the feature solely by himself.\r\n\
\r\n\
6.Terminations\r\n\
a.This License is effective until terminated.\r\n\
b.The rights and permissions under this License will terminate automatically without notice from the Company if you fail to comply with any term(s) or condition(s) of this License. Upon the termination of this License, you shall cease all use of the Software, and destroy all copies, full or partial, of the Software.\r\n\
\r\n\
7.Trademarks\r\n\
a.This License does not grant permission to use the trade names, trademarks, service marks of the Company, even if such marks are included within the Software.\r\n\
b.Neither the name of the Company, its trademarks, nor the names of its contributors may be used to endorse, advertise or promote products which are derived from, developed with, or use the Software, without prior written permission from the Company.\r\n\
\r\n\
8.Patents\r\n\
a.The Company grants to the Licensee a worldwide, royalty-free, non-exclusive, non-sublicensable license, under patent claims owned or controlled by the Company that are embodied within the Software, for the duration of this License, to use the Software. \r\n\
\r\n\
9.Disclaimer of Warranty\r\n\
a.Except when otherwise stated in writing, the Company provide the program “as is” without warranty of any kind, either expressed or implied, including, but not limited to, the implied warranties of merchantability and fitness for a particular purpose, to the extent permitted by applicable law\r\n\
\r\n\
10.Limitation of Liability\r\n\
a.To the extent permitted under Law, The Software is provided under an AS-IS basis. The Company shall never, and without any limit, be liable for any damage, cost, expense or any other payment incurred by Licensee as a result of the Software’s actions, failure, bugs and/or any other interaction between The Software  and Licensee’s end-equipment, computers, other software or any 3rd party, end-equipment, computer or services.  \r\n\
b.The Licensee shall be solely liable to any damage, defect or loss incurred as a result of operating the Software and undertake the risks contained in running the Software.\r\n\
\r\n\
11.Interpretation of sections 9 and 10\r\n\
a.If the disclaimer of warranty and limitation of liability provided above cannot be given local legal effect according to their terms, reviewing courts shall apply local law that most closely approximates an absolute waiver of all civil liability in connection with the Software.\r\n\
\r\n\
12.Applicable Law\r\n\
a.Where the Company indicates that this License relates to a contract for HM Government it will be subject to the latest issue of the relevant Government standard terms and conditions of contract which shall prevail in the event of conflict with the terms and conditions of this License.\r\n\
b.Unless otherwise agreed in writing by the parties, the License shall be governed by English law and be subject to the non-exclusive jurisdiction of the English Courts.\r\n\
c.The remedies provided herein are in addition to and not in substitution for any rights or remedies which may be available to the Company from time to time, whether express or implied under statute or common law.\r\n\
d.If any provision of this License, or the application thereof, shall for any reason and to any extent be determined by a court of competent jurisdiction to be invalid or unenforceable under applicable law, the remaining provisions of this License shall be interpreted so as best to reasonably effect the intent of the parties. The parties further agree to replace any such invalid or unenforceable provisions with valid and enforceable provisions designed to achieve, to the extent possible, the business purposes and intent of such invalid and unenforceable provisions.";
    

    m_AboutText.SetWindowTextW(AboutText);

    return TRUE;
}