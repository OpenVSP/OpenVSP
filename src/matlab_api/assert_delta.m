function assert_delta(val, val_ideal, val_tol, varargin)

    delta = abs(val - val_ideal);
    if nargin==3
        assert( delta < val_tol );
    else
        assert( delta < val_tol, varargin{1});
    end
