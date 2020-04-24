<template>
<b-nav id="sidebar" vertical class="d-lg-block d-none">
    <div class="brand" style="grid-row-start: 1;">
        <b-link to="/"><img src="../assets/img/pxc_logo_plain.png"></b-link>
    </div>
    <b-nav-item to="/">
        <div>
            <img src="../assets/img/hart_mux_outline.png">
            <p>Gateways</p>
        </div>
    </b-nav-item>
    <b-nav-item>
        Something Else
    </b-nav-item>
</b-nav>
</template>

<style scoped lang="scss">
@import '../assets/scss/_vars.scss';

#sidebar {
    position: fixed;
    top: 0;
    left: 0;
    padding: 0;
    background-color: $primary-color;
    height: 100%;
    min-width: 16rem;

    .brand {
        padding: 2rem;
        width: 100%;
        text-align: center;

        img {
            max-height: 3rem;
            min-width: 8rem;
        }
    }

    .nav-item {
        display: flex;
        flex: 1;
        border-top: 1px solid darken($primary-color, 5);

        &:last-child {
            border-bottom: 1px solid darken($primary-color, 5);
        }

        .nav-link {
            display: flex;
            flex: 1;
            align-items: center;
            height: 16rem;
            padding: 1rem;
            justify-content: center;
            color: #000;

            div {
                text-align: center;
            }

            img {
                max-height: 8rem;
                max-width: 8rem;
            }

            p {
                font-size: 1.5rem;
                line-height: 1.5em;
                margin-bottom: 0;
                clear: both;
            }

            &:hover {
                background-color: darken($primary-color, 5);
            }
        }
    }
}
</style>