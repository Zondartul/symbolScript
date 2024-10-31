//  commands.txt version 6
//  based on what's going on in pre-interpreter times
//  from step-2_me_16_Mar_2023
//  into 05_Dec_2023

//"-g", "two_wire2.msh",   /// two small wires with 1m between them and 5m around
//"-f", "vak_trans",       /// problem formulation: transient solution
//"-t", "-v", 
/// ---------- t: enable run-time tracing
/// ---------- v: use managed variables
//"-s", "5",       /// use solver 5
opt["solver_type"] := 5;
//"--timesteps", "100",  /// number of snapshots in time
opt["n_timesteps"] := 10;//100;
//"-dt", "1",            /// d/dt artificial multiplier
opt["dt_mult"] := 1.0;//1.0e-3;//1.0;
//"--freq", "1e-2",      /// frequency of excitaiton current
opt["freq"] := 6.0;//5e-2;
//"--tmax", "2e2",       /// length of simulated time
opt["tmax"] := 0.1;//2e2;
//"-3",       /// ------ ????
//"--prec", "1e-20",     /// solve until this precision is reached
//opt["solver_prec"] := 1e-10; /// oh no, all I did was remove implicit JxW from F_cell ///1e-14;//1e-18; this is getting bad//1e-20; // almost converges, 6e-19 after 10k iters
opt["solver_prec"] := 1e-20;
//"--iters", "10000"     /// number of solver iterations.
opt["solver_iters"] := 10000;

/// --- accessible options:
//{"selected_poly", opt.selected_poly},
//{"btrace", opt.btrace},
//{"bvars", opt.bprint},
//{"solver_iters", opt.solver_iters},
//{"solver_prec", opt.solver_prec},
//{"sw1", opt.sw1},
//{"sw2", opt.sw2},
//{"sw3", opt.sw3},
//{"sw4", opt.sw4},
//{"tmax", opt.tmax},
//{"freq", opt.freq},
//{"dt_mult", opt.dt_mult},
//{"n_timesteps", opt.n_timesteps},
//{"solver_type", opt.solver_type},

str1 := "\n-------------------- make grid ----------------------";
str2 := "\n-------------------- init formulation ---------------";
str3 := "\n-------------------- setup system -------------------";
str4 := "\n-------------------- assemble system ----------------";
str5 := "\n-------------------- solve --------------------------";
str6 := "\n-------------------- output results -----------------\n\n\n";
str7 := "-------------------- done ---------------------------";


test_array_append:commands({
    test_array := array[];
    print("test 1"); print(test_array);
    test_array := test_array + array[1];
    print("test 2"); print(test_array);
    test_array := test_array + array[2];
    print("test 3"); print(test_array);
    test_array := array[10, 20, 30];
    test_array2 := array[40, 50, 60];
    test_array3 = test_array + test_array2;
    print("test 4"); 
    print(test_array);
    print(test_array2);
    print(test_array3);
    quit();
});


/// prog::run()
/// 1: make_grid()
print(str1);
//mesh1 :gmsh("gmsh/mesh5.msh");
mesh1:gmsh("gmsh/mesh6.msh");
//mesh1:gmsh("two_wire2.msh");
grid := mesh1;
lock_shape();

/// 2: init_formulation()
print(str2);
/// pf: vak_trans (1)
scale = 0.05;
//omega = 60.0;
mu0 = 1.256e-6;


shape_1c_3q_cont:assyShape(1, 3, "continuous");
shape_1c_1q_disc:assyShape(1, 1, "discrete");
shape_1c_1q_cell:assyShape(1, 1, "cell");
shape_cont = shape_1c_3q_cont;
shape_disc = shape_1c_1q_disc;
shape_1c_1q_cont:assyShape(1, 1, "continuous");
shape_1c_2q_disc:assyShape(1, 2, "discrete");

/// figure out the area 
calc_body_area:commands({
    args(in_body_idx);
    /// first just count the nodes
    F_cba1_local = vector[(body_id() == in_body_idx)];
    F_cba1_ass_c := assemble(F_cba1_local, shape_cont);
    F_cba1_ass_d := assemble(F_cba1_local, shape_disc);
    n_node_c := sum(F_cba1_ass_c);
    n_node_d := sum(F_cba1_ass_d);
    /// now multiply each node by jacobian*weight (i.e. area)
    F_cba2_local = vector[(body_id() == in_body_idx) * JxW];
    F_cba2_ass_c := assemble(F_cba2_local, shape_cont);
    F_cba2_ass_d := assemble(F_cba2_local, shape_disc);
    n_area1_c := sum(F_cba2_ass_c);
    n_area1_d := sum(F_cba2_ass_d);
    /// try to simplify by using vector element-wise multiplication
    F_cba3_local = F_cba1_local * JxW;
    F_cba3_ass_c := assemble(F_cba3_local, shape_cont);
    F_cba3_ass_d := assemble(F_cba3_local, shape_disc);
    n_area2_c := sum(F_cba3_ass_c);
    n_area2_d := sum(F_cba3_ass_d); /// should be equal to n_area1 
    /// compare also One and One_JxW
    F_one_local = vector[1.0];
    F_one_ass_c := assemble(F_one_local, shape_cont);
    F_one_ass_d := assemble(F_one_local, shape_disc);
    n_one_c := sum(F_one_ass_c);
    n_one_d := sum(F_one_ass_d);
    F_JxW_local = vector[JxW];
    F_JxW_ass_c := assemble(F_JxW_local, shape_cont);
    F_JxW_ass_d := assemble(F_JxW_local, shape_disc);
    n_JxW_c := sum(F_JxW_ass_c);
    n_JxW_d := sum(F_JxW_ass_d); 
    save_outputs(array[n_node_c, n_node_d, n_area1_c, n_area1_d, n_area2_c, n_area2_d, n_one_c, n_one_d, n_JxW_c, n_JxW_d], "calc_area.csv");
    Res = n_area1_c;
});


mat_default:material({
    rel_mu:const(1.0) -- "relative magnetic permeability";
    ju:const(0.0) -- "current density in Z direction";
    M:const(0.0) -- "magnetization vector (should be v[2] or v[3])";
    sigma:const(0.0) -- "specific electrical conductivity";
    f1:const(0.0) -- "test value for body 1";
});


mat_air:material({
    sigma:const(1e-12);
});

mat_copper:material({
    ju:const(1.0e6); //(wire_current_density_A_per_m2);
    sigma:const(60e6);
    f1:const(1.0);
});

mat_iron:material({
    rel_mu:const(2e6);
    sigma:const(10e6);
});

body_air:body({
    idx:integer(1);
    mat:material = mat_air;
});

body_wire:body({
    idx:integer(2);
    mat:material = mat_copper;
});

body_part:body({
    idx:integer(3);
    mat:material = mat_iron;
});


//test_array_append();

calc_body_area(2);
wire_I = 1.0 -- "current, amps";
wire_S = Res -- "area, m2"; 
wire_J = wire_I / wire_S  -- "current density, A/m2";
mat_copper["ju"] := wire_J;

F_local = vector[juN];
    juN = (1.0 / sigma)*ju*ac_wave*N*JxW;
        ac_wave = sin(omega*t);
            omega = 2.0*3.14*opt["freq"];

M_local = matrix[[gNgN / mu]];
    mu = mu0 * rel_mu;
Mdt_local = matrix[[1.0*sigma * NN]];


t := 0.0;

/// 3: setup_system()
print(str3);

sys:SystemRealTrans({shape = shape_cont;});
sys_1c:SystemRealTrans({shape = shape_1c_1q_cont;});
sys_disc:SystemRealTrans({shape = shape_disc;});
sys_disc_2c:SystemRealTrans({shape = shape_1c_2q_disc;});

make_zero_dofvec:commands({
    args(in_shape);
    mzdf_sys:SystemRealTrans({shape = in_shape;});
    res := to_dofVector(mzdf_sys["A"]); // change to mzdf_sys
});

/// ---- debug : try to calculate the gradients
///              of obvious functions
///              1' = 0
///              x' = 1
///              x2' = 2x
///

getGrads:commands({ /// I can't do this because I'd need to defer the evaluation of F_local
    args(F_gg);
    V_ass = assemble(F_gg, shape_cont); /// assemble wants it to be named Fsomething
    V_assd = assemble(F_gg, shape_disc);
    V_conv = convert(V_ass, "discrete", "copy");
    Vr = real(V_ass, sys);
    Vrd = real(V_assd, sys_disc);
    Vrc = real(V_conv, sys_disc);
    Res1 = grad(Vr);
    Res2 = grad(Vrd);
    Res3 = grad(Vrc);
});

unit_test_gradient:commands({
    One_local = vector[1.0];
    F_gg = One_local;
    One := assemble(F_gg, shape_1c_1q_cont);
    One1 = One / One;
    Oned := assemble(F_gg, shape_1c_2q_disc);
    Oned1 = Oned / Oned;
        /// -- getGrads(One_local) --- 
        F_gg = One_local;
        
        V_ass = assemble(F_gg, shape_1c_1q_cont); /// assemble wants it to be named Fsomething
        V_ass1 = V_ass / One;
        Vr = real(V_ass1, sys_1c);
        Res1 = grad(Vr);
    G1 := Res1;
        
        V_assd = assemble(F_gg, shape_1c_2q_disc);
        V_assd1 = V_assd / Oned;
        Vrd = real(V_assd1, sys_disc_2c);
        Res2 = grad(Vrd);
    G1d := Res2;

            //V_conv = convert(V_ass1, "discrete", "copy");
            //Vrc = real(V_conv, sys_disc);
            //Res3 = grad(Vrc);
        //G1c := Res3;

    X_local = vector[x];
    F_gg = X_local;
    X := assemble(F_gg, shape_1c_1q_cont);
    X1 = X / One;
    Xd := assemble(F_gg, shape_1c_2q_disc);
    Xd1 = Xd / Oned;
        /// -- getGrads(X_local) --- 
        F_gg = X_local;

        V_ass = assemble(F_gg, shape_1c_1q_cont); /// assemble wants it to be named Fsomething
        Vr0 = real(V_ass, sys_1c);
        Res0 = grad(Vr0);
    GXr := Res0;

        V_ass1 = V_ass / One;
        Vr = real(V_ass1, sys_1c);
        Res1 = grad(Vr);
    GX := Res1;

        V_assd = assemble(F_gg, shape_1c_2q_disc);
        V_assd1 = V_assd / Oned;
        Vrd = real(V_assd1, sys_disc_2c);
        Res2 = grad(Vrd);
    GXd := Res2;
        
            //V_conv = convert(V_assd, "discrete", "copy");
            //Vrc = real(V_conv, sys_disc);
            //Res3 = grad(Vrc);
            //GXc := Res3;

    X2_local = vector[x*x];
    F_gg = X2_local;
    X2 := assemble(F_gg, shape_1c_1q_cont);
    X21 = X2 / One;
    X2d := assemble(F_gg, shape_1c_2q_disc);
    X2d1 = X2d / Oned;
        /// -- getGrads(X2_local) --- 
        F_gg = X2_local;
        V_ass = assemble(F_gg, shape_1c_1q_cont); /// assemble wants it to be named Fsomething
        Vr0 = real(V_ass, sys_1c);
        Res0 = grad(Vr0);
    GX2r := Res0;
        
        V_ass1 = V_ass / One;
        Vr = real(V_ass1, sys_1c);
        Res1 = grad(Vr);
    GX2 := Res1;

        V_assd = assemble(F_gg, shape_1c_2q_disc);
        V_assd1 = V_assd / Oned;
        Vrd = real(V_assd1, sys_disc_2c);
        Res2 = grad(Vrd);
    GX2d := Res2;

            //V_conv = convert(V_ass1, "discrete", "copy");
            //Vrc = real(V_conv, sys_disc);
        //Res3 = grad(Vrc);
        //GX2c := Res3;

    //TV = array[One, X, X2];

    //grad_outputs = array[One, One1, G1, G1d, G1c, X, X1, GX, GXd, GXc, X2, X21, GX2, GX2d, GX2c, TV];
    grad_outputs = array[One, One1, Oned, Oned1, G1, G1d, X, X1, Xd, Xd1, GX, GXr, GXd, X2, X2d, X2d1, X21, GX2, GX2r, GX2d];
    //save_outputs(grad_outputs);
    //quit();
});

//zero_sysvec := sys["A"];
zero_dofvec := to_dofVector(sys["A"]);


/// 4: assemble_system
print(str4);
M_ass = assemble(M_local, shape_cont);
M := real(M_ass, sys);
Mdt_ass = assemble(Mdt_local, shape_cont);
Mdt := real(Mdt_ass, sys);
F_ass = assemble(F_local, shape_cont);

sys["M"] := M;
sys["Mdt"] := Mdt;

inv_ju = 1.0 * sigma / (N*JxW);
Fjui = vector[inv_ju];
Cjui := assemble(Fjui, shape_cont);

F_B = vector[body_id()];
BodyId := assemble(F_B, shape_1c_1q_cell);

F_JAir = vector[(body_id() == 1) * inv_ju];
F_JWire = vector[(body_id() == 2) * inv_ju];
F_JPart = vector[(body_id() == 3) * inv_ju];

C_JAir := assemble(F_JAir, shape_cont);
C_JWire := assemble(F_JWire, shape_cont);
C_JPart := assemble(F_JPart, shape_cont);


/// 5: solve()
dt := (opt["tmax"] - 0.0) / opt["n_timesteps"];
dtm := dt * opt["dt_mult"];
h := dtm;
A0 := zero_dofvec;

cur_shape = shape_cont;

solve:commands({
    args(arg_M, arg_A, arg_F);
    sys2:SystemRealTrans({
        //shape = shape_cont;
        shape = cur_shape;
    });
    sys2["M"] := real(arg_M, sys2);
    sys2["A"] := real(arg_A, sys2);
    sys2["F"] := real(arg_F, sys2);
    //applyBoundaryValues(sys2);
    solveCG(sys2);
    res := to_dofVector(sys2["A"]);
});

solve_zero_bc:commands({
    args(arg_M, arg_A, arg_F);
    sys2:SystemRealTrans({
        //shape = shape_cont;
        shape = cur_shape;
    });
    sys2["M"] := real(arg_M, sys2);
    sys2["A"] := real(arg_A, sys2);
    sys2["F"] := real(arg_F, sys2);
    applyBoundaryValues(sys2);
    solveCG(sys2);
    res := to_dofVector(sys2["A"]);
});


series_IWire := array[]; /// bugged
series_IPart := array[];
series_count := 0.0;

old_solver_dt:commands({
    //args(new_t);
    print(str5);
    //t := new_t + 0.1;


    /// ??? Ma = F -> (Ms*as+Mdt*adt) = F -> Ms*A0+Mdt*(A1/h-A0/h) = F -> Mdt*A1/h = F - Ms*A0 +Mdt*A0/h -> Mdt*A1 = h*(F - Fs0 + Fdt0) -> Mdt*A1 = F0
    ///                             or  F -> Ms*A1+Mdt*(A1/h-A0/h) = F -> (Ms+Mdt/h => M1)*A1 = (F + (Mdt*A0/h => Fdt) => F1) -> M1*A1 = F1 (this one)
    ///    to calculate instantenous current, pretend that the magnetic fields are statically as we calculated:
    ///    Ms*As = Fs; which is what we get if A1 = A0, d/dt = 0.
    ///    Ms*As = JuN = (1.0/sigma)*juI*N*JxW
    ///    Ms*As*sigma / (N*JxW) = juI;
    // from formulation:----
    //> juN = (1.0 / sigma)*ju*ac_wave*N*JxW; - here ju*ac_wave is the input current density (ju(t))
    //>    ac_wave = sin(omega*t);
    //>      omega = 2.0*3.14*opt["freq"];
    // ----------------------
    ///  so after A1 is calculated, we can just do
    ///  juI := Ms*A1*sigma / (N*JxW);
    /// ---- after we calculate juI, we will integrate it (or take average and multiply by 4 cells * (0.05m*0.05 m) (area per cell) = *0.01)
    //M := real(M_ass, sys);
    //Mdt := real(Mdt_ass, sys);
    F := real(F_ass, sys);
    //sys["M"] := M;
    //sys["Mdt"] := Mdt;
    sys["F"] := F;
    applyBoundaryValues(sys);

    Ms := to_dofMatrix(sys["M"]);
    Mdt := to_dofMatrix(sys["Mdt"]);
    M1 = Ms + (Mdt / h);
    F := to_dofVector(sys["F"]);
    Fdt := Mdt*A0/h;
    F1 := F + Fdt;

    As := A0;
    solve(Ms, As, F);
    As := res;

    A1 := A0;
    solve(M1, A1, F1);
    A1 := res;
    Adt := A1 - A0;
    A0 := A1;

    /// ?? formula?
    /// M1*A1 = F1
    /// Ms*A1 + Mdt*A1/h = 

    postproc_A2EB();
    Fs1 := Ms*A1;    
    JuI := Fs1*Cjui;
    JAir := Fs1*C_JAir;
    JWire := Fs1*C_JWire;
    JPart := Fs1*C_JPart;
    IAir := sum(JAir);
    IWire := sum(JWire);
    IPart := sum(JPart);
    /// fuck, we can't set a var to itself
    /// because every time, it's created anew (blank) of type (function)
    /// and then assigned a value...
    print("--------- I test ------");
    print("--- before asn ---");
    print_noeval(IWire);
    print_noeval(series_IWire);
    print(IWire);
    series_IWire := series_IWire + array[IWire];
    print("--- after asn ---");
    print_noeval(IWire);
    print_noeval(series_IWire);
    print("--- dbg done ---");
    print(series_IWire);
    series_IPart := series_IPart + array[IPart];
    series_count := series_count + 1.0;
    time = t;
    //save_outputs(array[IAir, IWire, IPart, time], "measure.csv");

    output();
});

/// 6: output_results()
output:commands({
    print(str6);
    //outputs = array[A1, As, F, Fdt, F1, Ez, gA, gdA, Bvec, JuI, BodyId, JAir, JWire, JPart];
    //save_outputs(outputs);
    print("output(): save solution.vtk");
    save_outputs(array[A1, F, Ez, Bvec], "solution_"+sweep_I+"_"+timestep+".vtk");
    print("output(): save sln_extras.vtk");
    save_outputs(array[As, Fdt, F1, gA, gdA, BodyId, JAir, JWire, JPart], "sln_extras_"+sweep_I+"_"+timestep+".vtk");
});


postproc_A2EB:commands({
    Ez = Adt*(-1.0);
    rA1 = real(A1, sys);
    gA = grad(rA1);
    dA = convert(A1, "discrete", "copy");
    rdA = real(dA, sys_disc);
    gdA = grad(rdA);
    B = curl(rA1);

    Avec = array[zero_dofvec, zero_dofvec, A];
    /// WAIT
    /// CURL IS WRONG
    /// it takes grad(grad(A).xyz).xyz
    /// when actually it should be
    /// grad(grad([0,0,Az]))
    /// because A is not actually a scalar
    /// but a Az

    /// todo: try to pass B directly instead of making Bvec
    Bx = to_dofVector(B[0]);
    By = to_dofVector(B[1]);
    Bz = zero_dofvec; //to_dofVector(B[2]);
    Bvec = array[Bx, By, Bz];
    
    /// Bz = 0
    /// if we could group [Bx, By, Bz]
    /// we'd get vector view in paraview
});


post_currents:commands({
    //print(series_IWire);
    //print(series_IPart);
    nonzero(series_IWire);
    nonzero(series_IPart);
    save_outputs(array[series_IWire, series_IPart], "measure_"+sweep_I+".csv");
    IW_avg := avg(series_IWire);
    IW_amp := fft(series_IWire, omega);
    IP_avg := avg(series_IPart);
    IP_amp := fft(series_IPart, omega);
    Iratio := IW_amp / IP_amp;
    csv_name := "currents_"+sweep_I+"_"+timestep+".csv"; 
    save_outputs(array[IW_avg, IW_amp, IP_avg, IP_amp, Iratio], csv_name);
});

my_sweep:commands({
    sweep_from := 0.0;
    sweep_to := -2.0;
    sweep_step := -0.5;
    sweep_I := 0;
    n_sweeps := (sweep_to - sweep_from) / sweep_step;
    sweep_cur := sweep_from;
    sweep_param = 10.0^sweep_cur;
    
    did_loop = 0;

    sweeps := array[];//array["sweep_I", "n_sweeps", "sweep_cur", "sweep_param"];
    while(sweep_I < n_sweeps, {
        print("----------\n");
        print("sweep ");
        print(sweep_I);
        print("out of");
        print(n_sweeps);

        did_loop = 1;
        sweep_cur := sweep_from + sweep_step*sweep_I;
        opt["dt_mult"] := sweep_param;
        n_trans := 10;
        //@progress @quiet to supress output and print a progress bar instead
        my_solve_transient(n_trans);
        post_currents();
        sweeps := sweeps + array[sweep_I, n_sweeps, sweep_cur, sweep_param];
        sweep_I := sweep_I + 1;
    });
    save_outputs(sweeps, "sweeps.csv");
    print(did_loop);
    print(sweep_I);
});

my_solve_transient:commands({
    t := 0.0;
    dt := (opt.tmax) / (opt.n_timesteps);
    print(t); 
    print(opt.tmax);
    print(opt.n_timesteps);
    print(dt);
    timestep := 0;
    while(t < opt["tmax"], {
        print("--- iter ---");
        print("iteration t");
        print(t);
        print("out of");
        print(opt["tmax"]);
        old_solver_dt();
        t := t + dt;
        timestep := timestep + 1;
    });
    print(series_count);
});

solver_dt:commands({
    //my_sweep();
    t := 0.3;
    timestep := 3;
    //ass_matrix_test();
    gridding_test();
    quit();
});

/// assemble matrix complete

ass_matrix_test:commands({
    F_dummy = vector[0];

    F_amt_local = vector[ju]; // pick something that changes over space, maybe the source current
    F_amt_old = assemble(F_amt_local, shape_cont);
    
    M_ass = assembly_matrix(shape_cont); // 1 local DoF over this shape
    ///     assembly_matrix: shape includes n_q_points
    ///     output will have n_components * n_q_points * n_cells columns
    ///                             (call this 'n_global_q_points')
    ///     and n_global_dofs rows.
    ///     matrix vals are the test_f * JxW portion of the equation
    ///                             |
    ///                     i.e. N(i,real_q_pt) * JxW(real_q_pt) (a tutorial likened JxW to dx when integrating, though it has to do with unit cell coordinate stretching)
    F_amt_dist = distribute(F_amt_local, shape_cont);
    ///     distribute: output will have n_global_q_points points
    ///                 result is f evaluated at real_q_pt
    ///                 which affects BasisIntegrals: N() and friends
    ///         how do we pass real_q_pt? dof_coord only has q index. 
    ///         the assembly function MUST call fe_values.reinit(cell); on each cell
    ///         as well as vm->set_pfc(fe_values)
    ///         ... which is why we can't access the field at arbitrary coords without context
   

    F_amt_new = M_ass * F_amt_dist;
    time_new_before := time_compute(F_amt_new);
    F_amt_new = M_ass * F_amt_dist;
    print_noeval(F_amt_old);
    time_old := time_compute(F_amt_old);
    print_noeval(F_amt_old);
    time_ass := time_compute(M_ass);
    time_dist:= time_compute(F_amt_dist);
    print_noeval(F_amt_new);
    time_new := time_compute(F_amt_new);
    print_noeval(F_amt_new);
    //save_outputs(array[F_amt_old, F_amt_new], "ass_matrix_ju.csv");
    save_outputs(array[F_amt_old, F_amt_new, F_amt_dist], "ass_vectors.vtk");
    save_outputs(M_ass, "ass_matrix.csv");
    //save_outputs(F_amt_old, "ass_matrix_amt_old.csv");
    //save_outputs(F_amt_new, "ass_matrix_amt_new.csv");
    save_outputs(array[time_old, time_ass, time_dist, time_new, time_new_before], "ass_time.csv");
    print(size(F_amt_old));
    print(size(M_ass));
    print(size(F_amt_dist));
    print(size(F_amt_new));
});

body_center:commands({
    args(in_body_idx);
    bc_is_body = 1.0 * (body_id() == in_body_idx);
    Fl_bc_x = vector[bc_is_body * x];
    Fl_bc_y = vector[bc_is_body * y];
    Fl_bc_z = vector[bc_is_body * z];
    Fl_n_cells = vector[bc_is_body];
    Fg_bc_x := assemble(Fl_bc_x, shape_cont);
    Fg_bc_y := assemble(Fl_bc_y, shape_cont);
    Fg_bc_z := assemble(Fl_bc_z, shape_cont);
    Fg_n_cells := assemble(Fl_n_cells, shape_cont);
    
    n_cells := sum(Fg_n_cells);
    bc_x := sum(Fg_bc_x) / n_cells;
    bc_y := sum(Fg_bc_y) / n_cells;
    bc_z := sum(Fg_bc_z) / n_cells;
    res := array[bc_x, bc_y, bc_z];
});

compute_ref_field:commands({
    body_center(2);
    print_noeval(res);
    wire_coords := res;
    print(wire_coords);

    d_x = abs(wire_coords[0] - x);
    d_y = abs(wire_coords[1] - y);
    d_z = abs(wire_coords[2] - z);
    d = sqrt(d_x*d_x + d_y*d_y + d_z*d_z);
    //f = 1/(d*d);
    f = 1.0 / (d_x*d_x + d_y*d_y);
    Fl = vector[f];
    Fg := assemble(Fl, shape_cont);
    res := Fg;
});

/// next up: gridding test
/// step 1. measure field at a point
///   1.1: sample(F_global, array[x,y,z])->float using FEPointEvaluation()
/// step 2. make sure it's some kind of 1/r2 field (see electrostatic formulation)
/// step 3. change grid and try again
/// step 4. plot field

gridding_test:commands({
    //F1 = 1.0/(x*x);
    print("gridding_test begin");
    F1 = 1.0 * N * f1 * JxW;
    F_dummy_local = vector[F1];
    F_dummy_global := assemble(F_dummy_local, shape_cont);
    //F_Xsl_1 = vector[1/(x*x)];
    //Xsg_1 := assemble(F_Xsl_1, shape_cont);
    //compute_ref_field();
    //Xsg_1 := res;

    // ---- compute ref field ----------
        body_center(2);
    print_noeval(res);
    wire_coords := res;
    print(wire_coords);

    d_x = abs(wire_coords[0] - x);
    d_y = abs(wire_coords[1] - y);
    d_z = abs(wire_coords[2] - z);
    d = sqrt(d_x^2 + d_y^2 + d_z^2);
    //f = 1/(d*d);
    f = 1.0 * f_ref / (d^2);//sqrt(d_x^2 + d_y^2);
    Fl = vector[f];
    //---------------------------

    M1 = NN * JxW;
    M_local = matrix[[M1]];
    M_global := assemble(M_local, shape_cont);
    nonzero(M_global);

    //A0 := zero_dofvec;
    //A := A0;
    make_zero_dofvec(shape_cont);
    A := res;
    solve_zero_bc(M_global,A,F_dummy_global);
    A := res;
    nonzero(A);

    pt_peak = array[0.075,0.175,0];
    f_peak := sample(A, pt_peak);
    print(f_peak);
    // no longer related to f_peak
    f_ref = 1.0;
    Ref := assemble(Fl, shape_cont);
    nonzero(Ref);

    F_ones = vector[1.0];
    Fg_ones = assemble(F_ones, shape_cont);
    B := abs(A);// + (Fg_ones * 1e-20);
    //C := B / Xsg_1;
    C := B / Ref;

    //ptA = array[1.0,0.0,0.0];
    //ptB = array[2.0,0.0,0.0];
    //ptC = array[4.0,0.0,0.0];

    //valA := sample(F_dummy_global, ptA);
    //valB := sample(F_dummy_global, ptB);
    //valC := sample(F_dummy_global, ptC);

    //save_outputs(array[F_dummy_global, A, B, C, Xsg_1], "gridding_test.vtk");
    //save_outputs(array[valA, valB, valC], "gridding_test.csv");

    //-------------------------- old shape ----------------------
    //Fl_subdiv = vector[1.0 * ((x < 0.075) and (y < 0.1))];
    Fl_subdiv = vector[1.0 * ((x < 0.05) and (y < 0.1))];
    
    Fg_subdiv := assemble(Fl_subdiv, shape_cont);
    save_outputs(array[F_dummy_global, A, B, Ref, C, Fg_subdiv], "gridding_test.vtk");
    
    mesh2 := subdivide(mesh1, Fg_subdiv);
    print("END OLD SHAPE --------------------------");
    unlock_shape();
    grid := mesh2;
    lock_shape();
    shape_cont_2:assyShape(1, 3, "continuous");
    cur_shape = shape_cont_2;
    print("BEGIN NEW SHAPE ------------------------\n\n\n\n\n");
    //--------------------------- new shape --------------
    /// we can't refer to any old vectors or matrices or they'll pollute us with old assy reference
    Fd2 = vector[F1];
    Ml2 = matrix[[M1]];
    Fg2 := assemble(Fd2, shape_cont_2);
    Mg2 := assemble(Ml2, shape_cont_2);
    //F_Xsl_2 = vector[1/(x*x)];
    //Xsg_2 := assemble(F_Xsl_2, shape_cont_2);
    //Ref := assemble(Fl, shape_cont_2);


    //A0 := zero_dofvec;
    //A2 := A0;
    make_zero_dofvec(shape_cont_2);
    A2 := res;
    solve_zero_bc(Mg2,A2,Fg2);
    A2 := res;
    nonzero(A2);

    /// lining up the reference line with field graph:
    /// measure field at reference distance
    /// then scale the reference line by that idk
    //pt_peak = array[0.075,0.175,0];
    pt_ref = array[0.075,0.150,0];
    f_ref := sample(A2, pt_ref); // does this sample the wrong field? wut?
    print("f_ref = sample(A2, pt_ref) = "); print(f_ref);
    d_ref := 0.175 - 0.150;
    f_ref := f_ref * (d_ref^4); 
    
    Ref := assemble(Fl, shape_cont_2);
    nonzero(Ref);

    B2 := abs(A2);// + (Fg_ones * 1e-20); // polluted
    //C2 := B2 * Xsg_2;
    //save_outputs(array[Fg2, A2, B2, C2, Xsg_2], "gridding_test_2.vtk");
    save_outputs(array[Fg2, A2, B2, Ref], "gridding_test_2.vtk");
    print("gridding_test end");
});

/// next step (after assembly matrix is implemented) is to check if calculated field is
/// independent of gridding,
/// and preferrably make a function that checks symbolically if this is the case
/// for an arbitrary f_local (probably if it contains 1x JxW or more generally
/// a correct quadrature sum, as that is how integral is calculated. We want to be sure
/// that the integral of the field over space is independent of how many pieces
/// the space is split into.
/// grid-independence test can be done using a basic 1/R^2 field such as electrostatic one.
/// we check if E(4 meters) = (1/2)E(2 meters) = (1/4)E(1 meter) in one direction,
/// and same in the opposite direction where we use 2x or 3x the grid density.

/// PST: Grid-independence test confirms GRID DENSITY MATTERS
/// how do we fix it? field falls off twice as fast at subdivision 1.
/// try: subdivide only part of the domain (to leave the source the same) to verify further
/// reasons: JxW needed in both M and F, but I have those in commands.c, so why? (note that assy and pF doesn't touch JxW)
/// ... also, maybe plot convergence graph and also
///     the analytical solution to poissons eq is (1/(2pi))*ln|x-x0| + C

/// After the grid-independence test (if we verify or manage to fix it) we can:
/// choice:
///  A) "todo" parts of the todo.h
///  B) make a dummy nonlinear load such as 1/x or e^x for resistance and implement the harmonic balance method
///  C) split this huge commands.c file into several files and/or user functions that introduce new variables
///     and also hide local variables for generally better modularity and readability of code i.e.
///     so I can actually tell wtf commands.c is trying to do and where to poke next (needs void main())
///  D) learn Julia and re-do everything in there
///  E) make the scrappy InductionScript into a true programming language?
///     while simultaneously working on an underlying parse-anything framework?
///  F) put it all in a propper git folder
///  X) all of the above

/// done
print(str7);
