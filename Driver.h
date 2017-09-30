//
//  Driver.h
//  llvm
//
//  Created by Nicola Cabiddu on 21/09/2017.
//  Copyright © 2017 Nicola Cabiddu. All rights reserved.
//

#ifndef Driver_h
#define Driver_h


namespace driver {
   
   struct DriverConfiguration {
      
      bool enableJit_;
      bool enableOpt_;
      bool enableDebug_;
      
      bool saveAsObjectFile_;
      bool saveAsAsmFile_;
      bool saveAsIRFile_;
      bool dumpOnScreen_;
      
      explicit DriverConfiguration(bool enableJit = false,
                                   bool enableOpt = false,
                                   bool enableDebug = false,
                                   bool saveAsObjectFile = false,
                                   bool saveAsAsmFile = false,
                                   bool saveAsIRFile = false,
                                   bool dumpOnScreen = true);
      
   };
   
   ///
   /// @brief: compiler driver. it basically launches the parser, that return an ast that it used to
   ///         generate IR. Optionally this IR can be Jit compiled (than interpreted) or optimized further
   ///         if certain options are selected
   ///         By default the parser is launched and the IR printed on standard output
   ///
   class Driver
   {
   private:
      DriverConfiguration cnf_;

   public:
      
      Driver(DriverConfiguration cnf);
      void go();
   };
   
}


#endif /* Driver_h */
