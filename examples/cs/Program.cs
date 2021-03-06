﻿using System;
using fgDotNet;

namespace fgExample_cs
{
  static class Program
  {
    /// <summary>
    /// The main entry point for the application.
    /// </summary>
    [STAThread]
    static void Main()
    {
#if DEBUG
      var root = new Root("fgDirect2D_d.dll");
#else
      var root = new Root("fgDirect2D.dll");
#endif
      root.IterateControls(x => Console.Out.WriteLine(x));
      //fgRegisterFunction("statelistener", [](fgElement* self, const FG_Msg*) { fgElement* progbar = fgRoot_GetID(fgSingleton(), "#progbar"); progbar->SetValueF(self->GetValueF(0) / self->GetValueF(1), 0); progbar->SetText(StrF("%i", self->GetValue(0))); });
      //fgRegisterFunction("makepressed", [](fgElement* self, const FG_Msg*) { self->SetText("Pressed!"); });

      using (Layout layout = new Layout())
      {
        layout.LoadFileXML("../media/feathertest.xml");
        root.LayoutLoad(layout);

        root.GetID("#tabfocus").Action();

        while (root.ProcessMessages()) ;
      }
    }
  }
}
